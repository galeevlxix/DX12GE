#pragma once
#include "Behavior.h"
#include "Composites.h"
#include "Decorators.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>


class UtilityFactor {
public:
    virtual ~UtilityFactor() = default;
    virtual float calculate(Object3DNode* owner, Blackboard& blackboard) = 0;
    virtual std::unique_ptr<UtilityFactor> Clone() const = 0;
};

// Constant Factor
class ConstantFactor : public UtilityFactor {
    float m_Value;
public:
    ConstantFactor(float val) : m_Value(val) {}
    float calculate(Object3DNode* owner, Blackboard& blackboard) override {
        return m_Value;
    }
    std::unique_ptr<UtilityFactor> Clone() const override {
        return std::make_unique<ConstantFactor>(m_Value);
    }
};

// Blackboard Factor (reads a float/int from BB and normalizes it)
enum class CurveType { Linear, Logistic, Logit, Sine, Polynomial };

class BlackboardFactor : public UtilityFactor {
    std::string m_Key;
    float m_Min;
    float m_Max;
    CurveType m_Curve;
    float m_Slope;     // For Linear
    float m_Exponent;  // For Poly
    float m_Shift;     // For Logistic

public:
    BlackboardFactor(std::string key, float minVal, float maxVal, CurveType curve = CurveType::Linear)
        : m_Key(key), m_Min(minVal), m_Max(maxVal), m_Curve(curve), m_Slope(1.0f), m_Exponent(1.0f), m_Shift(0.0f) {}

    float calculate(Object3DNode* owner, Blackboard& blackboard) override {
        if (!blackboard.Has(m_Key)) return 0.0f;
        
        float rawVal = 0.0f;
        try {
             rawVal = blackboard.Get<float>(m_Key);
        } catch (...) {
             return 0.0f;
        }
        
        float norm = (rawVal - m_Min) / (m_Max - m_Min);
        norm = std::clamp(norm, 0.0f, 1.0f);

        switch (m_Curve) {
            case CurveType::Linear: return norm; 
            case CurveType::Logistic: return 1.0f / (1.0f + exp(-10.0f * (norm - 0.5f))); 
            default: return norm;
        }
    }
    
    std::unique_ptr<UtilityFactor> Clone() const override {
        return std::make_unique<BlackboardFactor>(m_Key, m_Min, m_Max, m_Curve);
    }
};


// ============================================================================
// DECORATOR
// ============================================================================
enum class Aggregation { Sum, Max, Min, Average, Multiply };

class UtilityDecorator : public Decorator {
    std::vector<std::shared_ptr<UtilityFactor>> m_Factors;
    Aggregation m_Aggr = Aggregation::Average;

public:
    UtilityDecorator(Behavior* child, Aggregation aggr = Aggregation::Average) 
        : m_Aggr(aggr) {
            if (child) setChild(std::unique_ptr<Behavior>(child));
        }

    void AddFactor(std::shared_ptr<UtilityFactor> factor) {
        m_Factors.push_back(std::move(factor));
    }

    float getUtility(Object3DNode* owner, Blackboard& bb) override {
        if (m_Factors.empty()) return 0.0f;

        float total = 0.0f;
        if (m_Aggr == Aggregation::Multiply) total = 1.0f;
        if (m_Aggr == Aggregation::Min) total = 99999.0f;
        if (m_Aggr == Aggregation::Max) total = -99999.0f;

        for (auto& f : m_Factors) {
            float val = f->calculate(owner, bb);
            switch (m_Aggr) {
                case Aggregation::Sum: total += val; break;
                case Aggregation::Multiply: total *= val; break;
                case Aggregation::Max: total = std::max(total, val); break;
                case Aggregation::Min: total = std::min(total, val); break;
                case Aggregation::Average: total += val; break;
            }
        }
        
        if (m_Aggr == Aggregation::Average) total /= m_Factors.size();
        return std::clamp(total, 0.0f, 1.0f);
    }

    BehaviorPtr Clone() const override {
        auto copy = std::make_unique<UtilityDecorator>(m_Child->Clone().release(), m_Aggr);
        for(auto& f : m_Factors) {
            copy->AddFactor(f->Clone());
        }
        return copy;
    }

protected:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override {
        return m_Child->tick(dt, owner, blackboard);
    }
};

// ============================================================================
// SELECTOR
// ============================================================================

class UtilitySelector : public Composite {
public:
    Status update(float dt, Object3DNode* owner, Blackboard& blackboard) override {
        float bestScore = -1.0f;
        Behavior* bestChild = nullptr;

        for (auto& child : m_Children) {
            float score = child->getUtility(owner, blackboard);
            if (score > bestScore) {
                bestScore = score;
                bestChild = child.get();
            }
        }

        if (bestChild) {
            return bestChild->tick(dt, owner, blackboard);
        }
        
        return Status::FAILURE;
    }

    BehaviorPtr Clone() const override {
        auto copy = std::make_unique<UtilitySelector>();
        for (auto& c : m_Children) {
            copy->addChild(c->Clone());
        }
        return copy;
    }
};
