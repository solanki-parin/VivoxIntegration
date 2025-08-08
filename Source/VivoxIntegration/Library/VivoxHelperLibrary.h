// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VivoxHelperLibrary.generated.h"

/**
 * 
 */
UCLASS()
class VIVOXINTEGRATION_API UVivoxHelperLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static FString GenerateUUID();
};
