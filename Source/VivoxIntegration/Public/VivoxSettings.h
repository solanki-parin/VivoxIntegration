// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "VivoxSettings.generated.h"


UENUM(BlueprintType)
enum class EVivoxAudioFadeModel : uint8
{
    /**
     *Fades voice quickly at first, buts slows down as you get further from conversational distance.
     */
    InverseByDistance = 0,
    /**
     *Fades voice slowly at first, but speeds up as you get further from conversational distance.
     */
    LinearByDistance,
    /**
     *Makes voice within the conversational distance louder, but fade quickly beyond it.
     */
    ExponentialByDistance
};


UCLASS(config = Game, defaultconfig)
class VIVOXINTEGRATION_API UVivoxSettings : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Vivox|PostionalChannel", meta = (UIMin = "1.0"))
	float AudioFadeIntensityByDistance=1.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Vivox|PostionalChannel", meta = (UIMin = "25"))
	float ConversationalDistance=90.0f;

	UPROPERTY(Config, EditAnywhere, Category = "Vivox|PostionalChannel", meta = (UIMin = "100"))
	float AudibleDistance = 2700;

	UPROPERTY(Config, EditAnywhere, Category = "Vivox|PostionalChannel")
    EVivoxAudioFadeModel AudioModel;
};
