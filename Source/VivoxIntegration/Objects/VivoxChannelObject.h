// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//Resource
#include "VivoxIntegration/Resource/VivoxResource.h"
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
	EVivoxChannelType CurrentChannelType;
	FString CurrentChannelSessionId;

public:

	ChannelId GetChannel();

	/*
	  Get SessionId of current channel
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|VoiceChannel", meta= (Keywords = "Id Session Channel"), BlueprintCosmetic)
	FString GetChannelSessionId() { return CurrentChannelSessionId; };

	/*
	  Get connection state of current channel
	*/
	UFUNCTION(BlueprintPure, Category = "Vivox|VoiceChannel",BlueprintCosmetic)
	ConnectionState GetChannelConnectionState();

	void JoinChannel(FString ChannelId, EVivoxChannelType ChannelType, FOnVivoxChannelJoined OnChannelJoined);

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
	  @param ForwardVector Actor Forrward Vector
	  @param UpVector Actor UpVector
	*/
	UFUNCTION(BlueprintCallable, Category = "Vivox|VoiceChannel|Positional", BlueprintCosmetic)
	void UpdateVivox3dPosition(const FVector& position, const FVector& ForwardVector, const FVector& UpVector);
};
