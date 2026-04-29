// Copyright (c) 2025 , SPD78. All rights reserved.


#include "Library/VivoxHelperLibrary.h"
#include "Misc/Guid.h"

FString UVivoxHelperLibrary::GenerateUUID()
{
	FGuid UUID = FGuid::NewGuid();
	return UUID.ToString();
}
