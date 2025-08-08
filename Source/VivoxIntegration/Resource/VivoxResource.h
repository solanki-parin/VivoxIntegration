// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IAudioDevice.h"
#include "VivoxResource.generated.h" 

//Logcat
DECLARE_LOG_CATEGORY_EXTERN(LogVivox, Log, All);
//

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnVivoxLoggedIn , bool,bLoginSuccessfull);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnVivoxChannelJoined, bool, bJoinSuccessfull);

USTRUCT(BlueprintType)
struct FVivoxCredentials
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Server;

	UPROPERTY(BlueprintReadWrite)
	FString Domain;

	UPROPERTY(BlueprintReadWrite)
	FString TokenIssuer;

	UPROPERTY(BlueprintReadWrite)
	FString TokenKey;
};

UENUM(BlueprintType)
enum class EVivoxChannelType : uint8
{
	NonPositional=0,
	Positional=1,
	Echo=2
};

UENUM(BlueprintType)
enum class EVivoxDeviceVoiceStatus: uint8
{
	Mute=0,
	UnMute=1
};

//Audio Device data for input output hardware 
USTRUCT(BlueprintType)
struct FAudioDeviceData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Name;
	
	UPROPERTY(BlueprintReadOnly)
	FString Id;

	FAudioDeviceData() {};

	FAudioDeviceData(const FString& InName, const FString& InId)
		: Name(InName), Id(InId) {
	}

	bool IsEmpty() const
	{
		return Name.IsEmpty() && Id.IsEmpty();
	}
};

// Class for AudioDevice Abstract class 
class UVivoxAudioDevice : public IAudioDevice
{
	FString DeviceName;
	FString DeviceId;

public:
	UVivoxAudioDevice(const FString& InName, const FString& InId)
		: DeviceName(InName), DeviceId(InId) {
	}

	virtual const FString& Name() const override
	{
		return DeviceName;
	}

	virtual const FString& Id() const override
	{
		return DeviceId;
	}

	virtual bool IsEmpty() const override
	{
		return DeviceName.IsEmpty() && DeviceId.IsEmpty();
	}
};
