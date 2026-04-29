// Copyright (c) 2025 , SPD78. All rights reserved.


#include "Objects/VivoxChannelObject.h"
//VivoxSettings
#include "VivoxSettings.h"
//
//Subsytem
#include "Subsystem/VivoxSubSystem.h"
//
#include "Kismet/KismetSystemLibrary.h"


void UVivoxChannelObject::SetAudioConnected(bool bListenAudio, bool bTransmitAudio)
{
	if (ChannelSession != nullptr)
	{
		ChannelSession->BeginSetAudioConnected(bListenAudio, bTransmitAudio);
		bTransmittingAudio = bTransmitAudio;
		bListeningAudio = bListenAudio;
	}
}

void UVivoxChannelObject::JoinChannel(FString ChannelSessionId, EVivoxChannelType ChannelType, FOnVivoxChannelJoined OnChannelJoined,bool bConnectAudio, bool bTransmitAudio)
{
	if (!(IsValid(GetOuter()) && GetOuter() != nullptr))
		return;

	if (!Cast<UGameInstance>(GetOuter()))
		return;

	UVivoxSubSystem* VivoxSubsystem = Cast<UGameInstance>(GetOuter())->GetSubsystem<UVivoxSubSystem>();

	if (!(IsValid(VivoxSubsystem) && VivoxSubsystem != nullptr))
		return;

	IChannelSession::FOnBeginConnectCompletedDelegate OnConnectionComplete;
	OnConnectionComplete.BindLambda([this, OnChannelJoined](VivoxCoreError Error)
		{
			OnChannelJoined.ExecuteIfBound(Error == 0);
		});
	VivoxSubsystem->LoginSession = &VivoxSubsystem->VivoxVoiceClient->GetLoginSession(VivoxSubsystem->LoggedInUserId);

	ChannelId Channel;
	UVivoxSettings* Setting = GetMutableDefault<UVivoxSettings>();
	Channel3DProperties PosChannelProperty = Channel3DProperties(Setting->AudibleDistance, Setting->ConversationalDistance,Setting->AudioFadeIntensityByDistance,StaticCast<EAudioFadeModel>(uint8(Setting->AudioModel)));
	UE_LOG(LogVivox,Warning,TEXT("The vivox position settings are , Audible distance %f , Convers %f , fadeInt %f."), Setting->AudibleDistance, Setting->ConversationalDistance, Setting->AudioFadeIntensityByDistance);
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
	ChannelSession->BeginConnect(bConnectAudio, false, bTransmitAudio, JoinToken, OnConnectionComplete);
	
	//No need for this in plugin here either make it somewhere else 

	ChannelSession->EventAfterParticipantAdded.AddLambda([this](const IParticipant& Participant)
		{
			UE_LOG(LogTemp, Log, TEXT("Participant added: %s"),
				*FString(Participant.Account().Name()));

			if (Participant.IsSelf())
			{
				UE_LOG(LogTemp, Log, TEXT("Local participant detected"));
				CurrentParticipant = const_cast<IParticipant*>(&Participant); // store pointer for later
			}
		});

	bTransmittingAudio = bTransmitAudio;
	bListeningAudio = bConnectAudio;
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
	if (!(IsValid(GetOuter()) && GetOuter()!=nullptr))
		return;

	if (!Cast<UGameInstance>(GetOuter()))
		return;

	UVivoxSubSystem* VivoxSubsystem = Cast<UGameInstance>(GetOuter())->GetSubsystem<UVivoxSubSystem>();

	if (!(IsValid(VivoxSubsystem) && VivoxSubsystem != nullptr))
		return;

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
	if (ChannelSession != nullptr)
	{
	 	auto AudioState = ChannelSession->AudioState();
		switch (AudioState)
		{
		case ConnectionState::Disconnected:
			UE_LOG(LogVivox, Warning, TEXT("Audio is not connected , cannot change position of speaker in 3d space"));
			break;
		case ConnectionState::Connecting:
			UE_LOG(LogVivox, Warning, TEXT("Audio is connecting , cannot change 3d postion for now"));
			break;
		case ConnectionState::Connected:
			CachedPosition.SetValue(position);
			CachedForwardVector.SetValue(ForwardVector);
			CachedUpVector.SetValue(UpVector);
			if (!Get3DValuesAreDirty())
				return;
			ChannelSession->Set3DPosition(CachedPosition.GetValue(), CachedPosition.GetValue(), CachedForwardVector.GetValue(), CachedUpVector.GetValue());
			Clear3DValuesAreDirty();
			break;
		case ConnectionState::Disconnecting:
			UE_LOG(LogVivox, Warning, TEXT("Cannot change 3d position the audio is disconnecting"));
			break;
		default:
			break;
		}	
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Channel Session is not valid cannot change position"));
	}
}

bool UVivoxChannelObject::IsSpeakingToChannel(double& AudioEnergy) const
{
	if (CurrentParticipant != nullptr)
	{
		AudioEnergy = CurrentParticipant->AudioEnergy();
		return CurrentParticipant->SpeechDetected();
	}
	AudioEnergy = 0.0f;
	return false;
}

