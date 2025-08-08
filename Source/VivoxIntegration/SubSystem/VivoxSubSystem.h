// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//Objects
#include "VivoxIntegration/Objects/VivoxChannelObject.h"
//
//Resource
#include "VivoxIntegration/Resource/VivoxResource.h"
//
//Vivox
#include "IClient.h"
#include "VivoxCore.h"
//
#include "VivoxSubSystem.generated.h"



UCLASS()
class VIVOXINTEGRATION_API UVivoxSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	

private:
	
	FVivoxCredentials Credentials;

public:

	//VivoxBasePropertySet
	IClient* VivoxVoiceClient;
	AccountId LoggedInUserId;

	//VivoxLoginPropertySet
	bool bIsLoggedIn = false;
	ILoginSession* LoginSession = nullptr;

	//Channel Objects

	TMap<FString, UVivoxChannelObject*> EchoChannels;
	TMap<FString, UVivoxChannelObject*> NonPostionalChannels;
	TMap<FString, UVivoxChannelObject*> PositionalChannels;

public:
	
	//Credentials

	/*
	  Sets vivox credentials , make sure to provide proper credentials otherwise vivox will not work
	  @param VivoxCredentials Vivox Credentials
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Credentials")
	void SetVivoxCredentials(const FVivoxCredentials& VivoxCredentials)
	{
		Credentials = VivoxCredentials;
	}

	/*
	  Gets creadentials of vivox integration
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Credentials")
	FVivoxCredentials GetVivoxCredentials() const
	{ 
		return Credentials;
	}

	//Base Vivox Functions

	/*
	  Initializes the vivox 
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox", BlueprintCosmetic)
	void InitializeVivox();

	/*
	  UnInitializes the vivox
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox", BlueprintCosmetic)
	void UnInitializeVivox();

	//Vivox Login Functions
	
	/*
	  Starts login to the vivox server
	  @param OnLogin Callaback event for login activity
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox", BlueprintCosmetic)
	void Login(FOnVivoxLoggedIn OnLogin);

	/*
	  Logout from the vivox server
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox", BlueprintCosmetic)
	void Logout();

	//Vivox Channel Functions
	
	/*
	  Creates and join the voice channel with specific ChannelSessionId 
	  @param ChannelSessionId Channel Id to identify Voice channel
	  @param ChannelType Voice Channel Type
	  @param OnChannelJoined Callback event for Voice Channel Creation activity
	  @param ChannelObject Channel object created using Channel Id
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|VoiceChannel", BlueprintCosmetic)
	void CreateAndJoinVoiceChannel(FString ChannelSessionId, EVivoxChannelType ChannelType, FOnVivoxChannelJoined OnChannelJoined, UVivoxChannelObject*& ChannelObject);

	/*
	  Gets all voice channel of specific type
	  @param ChannelType Voice Channel Type
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|VoiceChannel", BlueprintCosmetic)
	TArray<UVivoxChannelObject*> GetAllChannelOfType(EVivoxChannelType ChannelType) const;

	/*
	  Gets voice channel of type with ChannelSessionId
	  @param ChannelType Voice Channel Type
	  @param ChannelSessionId Channel Id
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|VoiceChannel", BlueprintCosmetic)
	UVivoxChannelObject* GetChannelOfType(EVivoxChannelType ChannelType ,FString ChannelSessionId) const;

	//Vivox Device functions

	/*
	  Mutes and Unmutes output device used by vivox 
	  @param Status Status to set Mute or UnMute
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", meta = (Keywords = "Mute Unmute Voice"),BlueprintCosmetic)
	void SetOutputDeviceVoiceState(EVivoxDeviceVoiceStatus Status);

	/*
	  Mutes and Unmutes input device used by vivox
	  @param Status Status to set Mute or UnMute
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", meta = (Keywords ="Mute Unmute Voice"), BlueprintCosmetic)
	void SetInputDeviceVoiceState(EVivoxDeviceVoiceStatus Status);

	/*
	  Gets state of output device used by vivox
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Mute Unmute Voice"), BlueprintCosmetic)
	EVivoxDeviceVoiceStatus GetOutputDeviceVoiceState();

	/*
	  Gets state of input device used by vivox
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Mute Unmute Voice"), BlueprintCosmetic)
	EVivoxDeviceVoiceStatus GetInputDeviceVoiceState();


	/*
	  Sets volume for output device used for vivox , Volume range is from 0 to 100, current 50
	  @param Volume Volume To Set
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", BlueprintCosmetic)
	void SetOutputDeviceVolume(int32 Volume=50);


	/*
	  Sets volume for input device used for vivox , Volume range is from 0 to 100, current 50
	  @param Volume Volume To Set
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", BlueprintCosmetic)
	void SetInputDeviceVolume(int32 Volume=50);

	/*
	 Sets Input Device to None (Stops capturing audio)
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", meta = (Keywords = "Input Voice Clear None Device"),BlueprintCosmetic)
	void SetInputDeviceToNone();

	/*
	 Sets Output Device to None (Stops playing audio)
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", meta = (Keywords = "Output Voice Clear None Device"), BlueprintCosmetic)
	void SetOutputDeviceToNone();

	/*
	 Gets Current Active Input device 
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Input Device"), BlueprintCosmetic)
	FAudioDeviceData GetActiveInputDevice();

	/*
	 Gets Current Active Output device
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Output Device"), BlueprintCosmetic)
	FAudioDeviceData GetActiveOutputDevice();

	/*
	 Sets Current Active Input device
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", meta = (Keywords = "Input Device"), BlueprintCosmetic)
	void SetActiveInputDevice(FAudioDeviceData DeviceData);

	/*
	 Sets Current Active Output device
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Device", meta = (Keywords = "Output Device"), BlueprintCosmetic)
	void SetActiveOutputDevice(FAudioDeviceData DeviceData);

	/*
	 Gets Default Communication Input device
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Input Device Communication"), BlueprintCosmetic)
	FAudioDeviceData GetInputCommunicationDevice();

	/*
	 Gets Default Communication Output device
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Output Device Communication"), BlueprintCosmetic)
	FAudioDeviceData GetOutputCommunicationDevice();

	/*
	 Gets the Effective Device ,If the active device is set to SystemDevice or CommunicationDevice, then the effective device shows the actual device used.If the active device is set to NullDevice or a physical device no longer connected to the system, then no device is effectively in use, and NullDevice is returned. If the active device is set to a specific physical device which is still connected to the system, then that device will be returned.
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Input Device Effective"), BlueprintCosmetic)
	FAudioDeviceData GetInputEffectiveDevice();

	/*
	  Gets the Effective Device ,If the active device is set to SystemDevice or CommunicationDevice, then the effective device shows the actual device used.If the active device is set to NullDevice or a physical device no longer connected to the system, then no device is effectively in use, and NullDevice is returned. If the active device is set to a specific physical device which is still connected to the system, then that device will be returned.
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Output Device Effective"), BlueprintCosmetic)
	FAudioDeviceData GetOutputEffectiveDevice();

	/*
	 Gets All Available Input devices
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Input Device"), BlueprintCosmetic)
	TMap<FString,FAudioDeviceData> GetAvailableInputDevices();

	/*
	 Gets Default Communication Output device
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|Device", meta = (Keywords = "Output Device"), BlueprintCosmetic)
	TMap<FString, FAudioDeviceData> GetAvailableOutputDevices();

	//Transmission Mode Functions

	/*
	 Sets Transmission mode to None (Listens to multiple channel but cannot speak to any channel)
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Transmission", meta = (KeyWords = "Transmission Voice Channel", ReturnDisplayName="Success"), BlueprintCosmetic)
	bool SetTransmissionToNone();

	/*
	 Sets Transmission mode to Specific Audio Channel (Listens to multiple channel but can only speak to provided channel)
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Transmission", meta = (KeyWords = "Transmission Voice Channel", ReturnDisplayName = "Success"), BlueprintCosmetic)
	bool SetTransmissionToSingleChannel(UVivoxChannelObject* ChannelObject);

	/*
	 Sets Transmission mode to All (Listens and Speaks to all Channel)
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|Transmission", meta = (KeyWords = "Transmission Voice Channel", ReturnDisplayName = "Success"), BlueprintCosmetic)
	bool SetTransmissionToAll();
};
