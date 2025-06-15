#include "../CurrentPass.h"

static CurrentPass::EPass Pass = CurrentPass::None;

CurrentPass::EPass CurrentPass::Get()
{
	return Pass;
}

void CurrentPass::Set(CurrentPass::EPass pass)
{
	Pass = pass;
}
