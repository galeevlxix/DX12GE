#pragma once

#include "../Base/DX12LibPCH.h"
#include "Node3D.h"

/// \brief Node class of spatial sound source in a scene. 
/// \code
/// // Example of creating:  
/// AudioEmitterNode emitter = new AudioEmitterNode();  
/// emitter->LoadWavRiff("project/resources/hit_sound.wav");
/// emitter->SpawnPlayingSound();
/// someNode->AddChild(emitter);
/// \endcode
class AudioEmitterNode : public Node3D
{
	X3DAUDIO_EMITTER m_EmitterData;
	SimpleMath::Vector3 m_PrevWorldPosition;
	std::vector<float> m_MatrixCoeffs;
	IXAudio2SourceVoice* m_Voice = nullptr;
	uint32_t m_AudioComponentId;

	float m_BaseVolume;
	float m_BasePitch;
	bool m_IsPlaying;

public:

	/// \brief Spawn and start playing the sound after it has been finished and destroyed.
	bool Loop;

	/// \brief The Doppler effect is enabled.
	bool DopplerEffect;

	/// \brief The sound from the source is heard everywhere at the same volume. The sound does not fade as it moves away from its source.
	bool Ubiquitous;

	AudioEmitterNode();

	virtual void OnUpdate(const double& deltaTime) override;

	/// \brief Loads audio component data from a Wav file.
	/// \throw std::runtime_error If wav file cannot be open.
	/// \throw std::runtime_error If wav format tag is unsupported.
	/// \throw std::runtime_error If reading data from the wav file failed.
	void LoadWav(const std::string& path);

	/// \brief Applies the loaded audio data and spawns the sound source.
	/// \param startPlay If it is true sound playback will start immediately.
	/// \return Returns true if sound spawning was successful.
	/// \return Returns false if audio component (wav data) is not initialized.
	/// \return Returns false if the sound source could not be created.
	/// \return Returns false if the startPlay parameter is true but playback could not be started.
	/// \see LoadWav
	bool SpawnPlayingSound(bool startPlay = true);

	/// \brief Stops playback and destroys the sound source.
	/// \return Returns true if sound destruction was successful. 
	/// \return Returns false if the sound was not spawned.
	/// \note This method does not destroy the node and its parameters. Use the Destroy method instead.
	/// \see Destroy
	bool DestroyPlayingSound();

	/// \brief Starts or continues audio playback.
	/// \return Returns true if playback started successfully. 
	/// \return Returns false if playback cannot be started.
	/// \note The sound must be spawned before playback is started. 
	/// \see SpawnPlayingSound
	bool StartPlayingSound();

	/// \brief Pauses audio playback.
	/// \return Returns true if playback paused successfully. 
	/// \return Returns false if playback cannot be paused.
	/// \note The sound must be spawned before playback is stopped. 
	/// \see SpawnPlayingSound
	bool StopPlayingSound();

	/// \brief Audio playback has finished.
	bool IsFinished();

	/// \brief Audio playback is currently running.
	bool IsPlaying() { return m_IsPlaying; }

	/// \brief Sets the volume level of the sound source. [0..32] 
	void SetVolume(float vol);

	/// \brief Returns the volume level of the sound source. 
	float GetVolume();

	/// \brief Sets the pitch level of the sound source. [0..32] 
	void SetPitch(float pitch);

	/// \brief Returns the pitch level of the sound source. 
	float GetPitch();

	/// \brief Returns the wav file of a audio component.
	const std::string GetWavFilePath();

	/// \brief Sets a new audio component (wav data) by id for this node.
	void SetComponentId(uint32_t newId);

	/// \brief Returns the id of an audio component (wav data) in the resource storage.
	uint32_t GetComponentId() { return m_AudioComponentId; }

	/// \brief Checks whether an audio component (wav data) is valid for playback.
	/// \return Returns false if wav data wav data has not been loaded. Returns true otherwise.
	/// \see LoadWav
	bool IsValid() { return m_AudioComponentId != -1; }

	virtual void Destroy(bool keepComponent = true) override;
	virtual Node3D* Clone(Node3D* newParent = nullptr, bool cloneChildrenRecursive = false, Node3D* cloneNode = nullptr) override;
	virtual void DrawDebug() override;
	virtual void CreateJsonData(json& j) override;
	virtual void LoadFromJsonData(const NodeSerializingData& nodeData) override;

private:	
	void InitPointEmitter(int srcChannels, int dstChannels);
	bool TreeHasEmittersWithComponentId(uint32_t id, Node3D* current = nullptr);
};