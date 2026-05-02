// Copyright (c) 2025 , SPD78. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//Resource
#include "Resource/VivoxResource.h"
//
//Vivox

#include "IClient.h"
#include "VivoxCore.h"
//
#include "VivoxChannelObject.generated.h"

//For Positional Channel
template<class T>
class CachedProperty
{
public:
	explicit CachedProperty(T value) {
		m_dirty = false;
		m_value = value;
	}

	const T& GetValue() const {
		return m_value;
	}

	void SetValue(const T& value) {
		if (m_value != value) {
			m_value = value;
			m_dirty = true;
		}
	}

	void SetDirty(bool value) {
		m_dirty = value;
	}

	bool IsDirty() const {
		return m_dirty;
	}
protected:
	bool m_dirty;
	T m_value;
};


UCLASS(BlueprintType)
class VIVOXINTEGRATION_API UVivoxChannelObject : public UObject
{
	GENERATED_BODY()
	
private:

	IChannelSession* ChannelSession = nullptr;

	//Channel property 
	EVivoxChannelType CurrentChannelType;
	FString CurrentChannelSessionId;

	//Currently not using
	bool bTransmittingAudio = false;
	bool bListeningAudio = false;

	//Participant 
	IParticipant* CurrentParticipant;

public:

	ChannelId GetChannel();

	/*
	  Get SessionId of current channel
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|VoiceChannel", meta= (Keywords = "Id Session Channel", ReturnDisplayName = "ChannelSessionId"), BlueprintCosmetic)
	FString GetChannelSessionId() { return CurrentChannelSessionId; };

	/*
	  Get connection state of current channel
	*/
	UFUNCTION(BlueprintPure, meta = (Keywords = "Channel", ReturnDisplayName = "ConnectionState"),Category = "Vivox|VoiceChannel",BlueprintCosmetic)
	ConnectionState GetChannelConnectionState();

	/*
	  Set the audio and transmission state for channel
	  @param bListenAudio True to add audio, false to remove audio.
	  @param bTransmitAudio When audio is added, transmit only into this channel. This overrides and changes the TransmissionMode set in ILoginSession. If transmission is specifically set to this channel, then when audio is removed, TransmissionMode changes to "None".
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|VoiceChannel", meta = (Keywords = "Audio Listen Transmission"), BlueprintCosmetic)
	void SetAudioConnected(bool bListenAudio=true,bool bTransmitAudio=true);

	void JoinChannel(FString ChannelId, EVivoxChannelType ChannelType , FOnVivoxChannelJoined OnChannelJoined, bool bConnectAudio = true, bool bTransmitAudio = true);

	/*
	  Leaves the current channel and destroys the object (Use CreateAndJoinChannelVoiceChannel from VivoxSubSystem to join the same channel again) 
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|VoiceChannel", BlueprintCosmetic)
	void LeaveChannel();

	//Positional Channel Property 

	CachedProperty<FVector> CachedPosition = CachedProperty<FVector>(FVector());
	CachedProperty<FVector> CachedForwardVector = CachedProperty<FVector>(FVector());
	CachedProperty<FVector> CachedUpVector = CachedProperty<FVector>(FVector());
	bool Get3DValuesAreDirty() const;
	void Clear3DValuesAreDirty();

	/*
	  Sets the position of player in 3d world space for positional channel
	  @param position Actor Location
	  @param ForwardVector Actor Forward Vector
	  @param UpVector Actor UpVector
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|VoiceChannel|Positional", meta = (Keywords = "Channel Position Location Transform"), BlueprintCosmetic)
	void UpdateVivox3dPosition(const FVector& position, const FVector& ForwardVector, const FVector& UpVector);

	//Used to check if currently speaking to channel or not 
	UFUNCTION(BlueprintPure, meta = (ReturnDisplayName = "IsSpeaking"),Category = "Vivox|VoiceChannel")
	bool IsSpeakingToChannel(double& AudioEnergy) const;

};
