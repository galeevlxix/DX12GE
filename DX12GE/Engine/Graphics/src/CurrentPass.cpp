#include "../CurrentPass.h"

CurrentPass::EPass CurrentPass::Get()
{
	return Pass;
}

void CurrentPass::Set(CurrentPass::EPass pass)
{
	Pass = pass;
}
