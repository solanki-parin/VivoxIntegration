// Fill out your copyright notice in the Description page of Project Settings.


#include "VivoxHelperLibrary.h"
#include "Misc/Guid.h"

FString UVivoxHelperLibrary::GenerateUUID()
{
	FGuid UUID = FGuid::NewGuid();
	return UUID.ToString();
}
