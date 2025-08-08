// Fill out your copyright notice in the Description page of Project Settings.


#include "VivoxChannelObject.h"
//VivoxSettings
#include "VivoxIntegration/Public/VivoxSettings.h"
//
//Subsytem
#include "VivoxIntegration/Subsystem/VivoxSubSystem.h"
//
#include "Kismet/KismetSystemLibrary.h"


void UVivoxChannelObject::JoinChannel(FString ChannelSessionId, EVivoxChannelType ChannelType, FOnVivoxChannelJoined OnChannelJoined)
{
	check(GetOuter());

	UVivoxSubSystem* VivoxSubsystem = Cast<UVivoxSubSystem>(GetOuter());

	check(VivoxSubsystem);

	IChannelSession::FOnBeginConnectCompletedDelegate OnConnectionComplete;
	OnConnectionComplete.BindLambda([this, OnChannelJoined](VivoxCoreError Error)
		{
			OnChannelJoined.ExecuteIfBound(Error == 0);
		});
	VivoxSubsystem->LoginSession = &VivoxSubsystem->VivoxVoiceClient->GetLoginSession(VivoxSubsystem->LoggedInUserId);

	ChannelId Channel;
	UVivoxSettings* Setting = GetMutableDefault<UVivoxSettings>();
	Channel3DProperties PosChannelProperty = Channel3DProperties(Setting->AudibleDistance, Setting->ConversationalDistance,Setting->AudioFadeIntensityByDistance,StaticCast<EAudioFadeModel>(uint8(Setting->AudioModel)));

	CurrentChannelType = ChannelType;
	switch (ChannelType)
	{
	case EVivoxChannelType::Positional:

		Channel = ChannelId(VivoxSubsystem->GetVivoxCredentials().TokenIssuer, ChannelSessionId, VivoxSubsystem->GetVivoxCredentials().Domain, ChannelType::Positional, PosChannelProperty);
		break;
	case EVivoxChannelType::NonPositional:
		Channel = ChannelId(VivoxSubsystem->GetVivoxCredentials().TokenIssuer, ChannelSessionId, VivoxSubsystem->GetVivoxCredentials().Domain, ChannelType::NonPositional);
		break;
	case EVivoxChannelType::Echo:
		Channel = ChannelId(VivoxSubsystem->GetVivoxCredentials().TokenIssuer, ChannelSessionId, VivoxSubsystem->GetVivoxCredentials().Domain, ChannelType::Echo);
		break;
	default:
		break;
	}
	ChannelSession = &VivoxSubsystem->LoginSession->GetChannelSession(Channel);
	FString JoinToken = ChannelSession->GetConnectToken(VivoxSubsystem->GetVivoxCredentials().TokenKey, FTimespan::FromSeconds(180));
	ChannelSession->BeginConnect(true, false, true, JoinToken, OnConnectionComplete);
	CurrentChannelSessionId = ChannelSessionId;
}

ChannelId UVivoxChannelObject::GetChannel()
{
	if (ChannelSession != nullptr)
	{
		return ChannelSession->Channel();
	}
	return ChannelId();
}

ConnectionState UVivoxChannelObject::GetChannelConnectionState()
{
	if (ChannelSession != nullptr)
	{
		auto ConnState = ChannelSession->ChannelState();
		switch (ConnState)
		{
		case ConnectionState::Disconnected:
			return ConnectionState::Disconnected;
		case ConnectionState::Connecting:
			return ConnectionState::Connecting;
		case ConnectionState::Connected:
			return ConnectionState::Connected;
		case ConnectionState::Disconnecting:
			return ConnectionState::Disconnecting;
		default:
			return ConnectionState::Disconnected;
		}
	}
	return ConnectionState::Disconnected;
}

void UVivoxChannelObject::LeaveChannel()
{
	check(GetOuter());

	UVivoxSubSystem* VivoxSubsystem = Cast<UVivoxSubSystem>(GetOuter());

	check(VivoxSubsystem);

	const FString* Key;
	switch (CurrentChannelType)
	{
	case EVivoxChannelType::NonPositional:
		Key = VivoxSubsystem->NonPostionalChannels.FindKey(this);
		if (Key)
		{
			VivoxSubsystem->NonPostionalChannels.Remove(*Key);
		}
		break;
	case EVivoxChannelType::Positional:
		Key = VivoxSubsystem->PositionalChannels.FindKey(this);
		if (Key)
		{
			VivoxSubsystem->PositionalChannels.Remove(*Key);
		}
		break;
	case EVivoxChannelType::Echo:
		Key = VivoxSubsystem->EchoChannels.FindKey(this);
		if (Key)
		{
			VivoxSubsystem->EchoChannels.Remove(*Key);
		}
		break;
	default:
		break;
	}

	if (ChannelSession != nullptr)
	{
		ChannelSession->Disconnect(false);
		ChannelSession = nullptr;
	}

	this->MarkAsGarbage();
	UKismetSystemLibrary::CollectGarbage();
}

//Vivox 3d position

bool UVivoxChannelObject::Get3DValuesAreDirty() const
{
	return (CachedPosition.IsDirty() || CachedForwardVector.IsDirty() || CachedUpVector.IsDirty());
}

void UVivoxChannelObject::Clear3DValuesAreDirty()
{
	CachedPosition.SetDirty(false);
	CachedForwardVector.SetDirty(false);
	CachedUpVector.SetDirty(false);
}

void UVivoxChannelObject::UpdateVivox3dPosition(const FVector& position, const FVector& ForwardVector, const FVector& UpVector)
{
	if (ChannelSession != nullptr && ChannelSession)
	{
		CachedPosition.SetValue(position);
		CachedForwardVector.SetValue(ForwardVector);
		CachedUpVector.SetValue(UpVector);
		if (!Get3DValuesAreDirty())
			return;
		ChannelSession->Set3DPosition(CachedPosition.GetValue(), CachedPosition.GetValue(), CachedForwardVector.GetValue(), CachedUpVector.GetValue());
		Clear3DValuesAreDirty();
		
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Channel Session is not valid cannot change position"));
	}
}
