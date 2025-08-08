// Fill out your copyright notice in the Description page of Project Settings.

#include "VivoxSubSystem.h"
#include "VivoxIntegration/Library/VivoxHelperLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UVivoxSubSystem::InitializeVivox()
{
	VivoxVoiceClient = &static_cast<FVivoxCoreModule*>(&FModuleManager::Get().LoadModuleChecked(TEXT("VivoxCore")))->VoiceClient();

	if (VivoxVoiceClient)
	{
		VivoxVoiceClient->Initialize();
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Failed to initialize vivox"));
	}
}

void UVivoxSubSystem::UnInitializeVivox()
{
	Logout();
	if (VivoxVoiceClient)
		VivoxVoiceClient->Uninitialize();
	VivoxVoiceClient = nullptr;
}

//Vivox Login Functions

void UVivoxSubSystem::Login(FOnVivoxLoggedIn OnLogin)
{
	check(Credentials.Domain != "" && Credentials.Server != "" && Credentials.TokenIssuer != "" && Credentials.TokenKey != "");

	FString Useruuid = UVivoxHelperLibrary::GenerateUUID();
	LoggedInUserId = AccountId(Credentials.TokenIssuer, Useruuid, Credentials.Domain);
	ILoginSession& LoginSessionVivox(VivoxVoiceClient->GetLoginSession(LoggedInUserId));
	LoginSession = &LoginSessionVivox;
	FTimespan TokenExpiration = FTimespan::FromSeconds(180);
	FString LoginToken = LoginSession->GetLoginToken(Credentials.TokenKey, TokenExpiration);
	ILoginSession::FOnBeginLoginCompletedDelegate OnBeginLoginCompleted;
	OnBeginLoginCompleted.BindLambda([this, OnLogin,&LoginSessionVivox](VivoxCoreError Error)
		{
			bIsLoggedIn = (Error == 0) ? true : false;
			OnLogin.ExecuteIfBound(Error==0);
		});
	LoginSession->BeginLogin(Credentials.Server, LoginToken, OnBeginLoginCompleted);
}

void UVivoxSubSystem::Logout()
{
	if (LoginSession)
	{
		{//Leaves from all the channels before logging out and clears the array

			 EchoChannels;
			 NonPostionalChannels;
			 PositionalChannels;
			 TArray<UVivoxChannelObject*> ObjArray;
			 EchoChannels.GenerateValueArray(ObjArray);
 			 for (UVivoxChannelObject* Obj : ObjArray)
			 {
				 const FString* Key = EchoChannels.FindKey(Obj);
				 if (Key)
				 {
					 EchoChannels.Remove(*Key);
					 Obj->LeaveChannel();
				 }
			 }
			 ObjArray.Empty();
			 NonPostionalChannels.GenerateValueArray(ObjArray);
			 for (UVivoxChannelObject* Obj : ObjArray)
			 {
				 const FString* Key = NonPostionalChannels.FindKey(Obj);
				 if (Key)
				 {
					 NonPostionalChannels.Remove(*Key);
					 Obj->LeaveChannel();
				 }
			 }
			 ObjArray.Empty();
			 PositionalChannels.GenerateValueArray(ObjArray);
			 for (UVivoxChannelObject* Obj : ObjArray)
			 {
				 const FString* Key = PositionalChannels.FindKey(Obj);
				 if (Key)
				 {
					 PositionalChannels.Remove(*Key);
					 Obj->LeaveChannel();
				 }
			 }
		}
		LoginSession->Logout();
		bIsLoggedIn = false;
		LoginSession = nullptr;
	}	
}

//Vivox Channel functions

void UVivoxSubSystem::CreateAndJoinVoiceChannel(FString ChannelSessionId, EVivoxChannelType ChannelType, FOnVivoxChannelJoined OnChannelJoined, UVivoxChannelObject*& ChannelObject)
{
	check(ChannelSessionId != "" && Credentials.Domain != "" && Credentials.TokenIssuer != "" && Credentials.TokenKey != "");

	if (LoggedInUserId.IsValid() && bIsLoggedIn)
	{
		UVivoxChannelObject* VivoxChObj = nullptr;
		switch (ChannelType)
		{
		case EVivoxChannelType::Positional:
			if (PositionalChannels.Contains(ChannelSessionId))
			{
				VivoxChObj = PositionalChannels[ChannelSessionId];
				VivoxChObj->JoinChannel(ChannelSessionId, ChannelType, OnChannelJoined);
			}
			else
			{
				VivoxChObj = NewObject<UVivoxChannelObject>(this);
				VivoxChObj->JoinChannel(ChannelSessionId, ChannelType, OnChannelJoined);
				PositionalChannels.Add(ChannelSessionId, VivoxChObj);
			}
			break;
		case EVivoxChannelType::NonPositional:
			if (NonPostionalChannels.Contains(ChannelSessionId))
			{
				VivoxChObj = NonPostionalChannels[ChannelSessionId];
				VivoxChObj->JoinChannel(ChannelSessionId, ChannelType, OnChannelJoined);
			}
			else
			{
				VivoxChObj = NewObject<UVivoxChannelObject>(this);
				VivoxChObj->JoinChannel(ChannelSessionId, ChannelType, OnChannelJoined);
				NonPostionalChannels.Add(ChannelSessionId, VivoxChObj);
			}
			break;
		case EVivoxChannelType::Echo:
			if (EchoChannels.Contains(ChannelSessionId))
			{
				VivoxChObj = EchoChannels[ChannelSessionId];
				VivoxChObj->JoinChannel(ChannelSessionId, ChannelType, OnChannelJoined);
			}
			else
			{
				VivoxChObj = NewObject<UVivoxChannelObject>(this);
				VivoxChObj->JoinChannel(ChannelSessionId, ChannelType, OnChannelJoined);
				EchoChannels.Add(ChannelSessionId, VivoxChObj);
			}
			break;
		default:
			break;
		}
		ChannelObject = VivoxChObj;
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Cannot create voice channel, because not logged in to vivox try login first"));
		OnChannelJoined.ExecuteIfBound(false);
	}
}

TArray<UVivoxChannelObject*> UVivoxSubSystem::GetAllChannelOfType(EVivoxChannelType ChannelType) const
{
	TArray<UVivoxChannelObject*> ChannelArray;
	switch (ChannelType)
	{
	case EVivoxChannelType::Positional:
		PositionalChannels.GenerateValueArray(ChannelArray);
		break;
	case EVivoxChannelType::NonPositional:
		NonPostionalChannels.GenerateValueArray(ChannelArray);
		break;
	case EVivoxChannelType::Echo:
		EchoChannels.GenerateValueArray(ChannelArray);
		break;
	default:
		return TArray<UVivoxChannelObject*>();
	}
	return ChannelArray;
}

UVivoxChannelObject* UVivoxSubSystem::GetChannelOfType(EVivoxChannelType ChannelType , FString ChannelSessionId) const
{
	switch (ChannelType)
	{
	case EVivoxChannelType::Positional:
		return PositionalChannels.FindRef(ChannelSessionId);;
	case EVivoxChannelType::NonPositional:
		return NonPostionalChannels.FindRef(ChannelSessionId);;
	case EVivoxChannelType::Echo:
		return EchoChannels.FindRef(ChannelSessionId);;
	default:
		return nullptr;
	}
}

//Vivox Device Functions

void UVivoxSubSystem::SetOutputDeviceVoiceState(EVivoxDeviceVoiceStatus Status)
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		switch (Status)
		{
		case EVivoxDeviceVoiceStatus::Mute:
			Device.SetMuted(true);
			break;
		case EVivoxDeviceVoiceStatus::UnMute:
			Device.SetMuted(false);
			break;
		default:
			Device.SetMuted(false);
			break;
		}
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
}

void UVivoxSubSystem::SetInputDeviceVoiceState(EVivoxDeviceVoiceStatus Status)
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		switch (Status)
		{
		case EVivoxDeviceVoiceStatus::Mute:
			Device.SetMuted(true);
			break;
		case EVivoxDeviceVoiceStatus::UnMute:
			Device.SetMuted(false);
			break;
		default:
			Device.SetMuted(false);
			break;
		}
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
}

EVivoxDeviceVoiceStatus UVivoxSubSystem::GetOutputDeviceVoiceState()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		return Device.Muted() ? EVivoxDeviceVoiceStatus::Mute : EVivoxDeviceVoiceStatus::UnMute;
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return EVivoxDeviceVoiceStatus::Mute;
}

EVivoxDeviceVoiceStatus UVivoxSubSystem::GetInputDeviceVoiceState()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		return Device.Muted() ? EVivoxDeviceVoiceStatus::Mute : EVivoxDeviceVoiceStatus::UnMute;
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return EVivoxDeviceVoiceStatus::Mute;
}

void UVivoxSubSystem::SetOutputDeviceVolume(int32 Volume)
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		Device.SetVolumeAdjustment(UKismetMathLibrary::MapRangeClamped(Volume, 0, 100, -50, 50));
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
}

void UVivoxSubSystem::SetInputDeviceVolume(int32 Volume)
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		Device.SetVolumeAdjustment(UKismetMathLibrary::MapRangeClamped(Volume,0,100,-50,50));
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
}

void UVivoxSubSystem::SetInputDeviceToNone()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		Device.SetActiveDevice(Device.NullDevice());
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
}

void UVivoxSubSystem::SetOutputDeviceToNone()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		Device.SetActiveDevice(Device.NullDevice());
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
}

FAudioDeviceData UVivoxSubSystem::GetActiveInputDevice()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		Device.Refresh();
		return FAudioDeviceData(Device.ActiveDevice().Name(), Device.ActiveDevice().Id());

	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return FAudioDeviceData();
}

FAudioDeviceData UVivoxSubSystem::GetActiveOutputDevice()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		Device.Refresh();
		return FAudioDeviceData(Device.ActiveDevice().Name(), Device.ActiveDevice().Id());

	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return FAudioDeviceData();
}

void UVivoxSubSystem::SetActiveInputDevice(FAudioDeviceData DeviceData)
{
	if (!DeviceData.IsEmpty())
	{
		if (VivoxVoiceClient)
		{
			auto& Device = VivoxVoiceClient->AudioInputDevices();
			Device.Refresh();
			UVivoxAudioDevice InputDevice(DeviceData.Name, DeviceData.Id);
			Device.SetActiveDevice(InputDevice);
		}
		else
		{
			UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
		}
		return;
	}
	UE_LOG(LogVivox, Warning, TEXT("Cannot set the device the provided device data is not valid"));
}


void UVivoxSubSystem::SetActiveOutputDevice(FAudioDeviceData DeviceData)
{
	if (!DeviceData.IsEmpty())
	{
		if (VivoxVoiceClient)
		{
			auto& Device = VivoxVoiceClient->AudioOutputDevices();
			Device.Refresh();
			UVivoxAudioDevice OutputDevice(DeviceData.Name, DeviceData.Id);
			Device.SetActiveDevice(OutputDevice);
		}
		else
		{
			UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
		}
		return;
	}
	UE_LOG(LogVivox, Warning, TEXT("Cannot set the device the provided device data is not valid"));
}

FAudioDeviceData UVivoxSubSystem::GetInputCommunicationDevice()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		Device.Refresh();
		return FAudioDeviceData(Device.CommunicationDevice().Name(), Device.CommunicationDevice().Id());
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return FAudioDeviceData();
}

FAudioDeviceData UVivoxSubSystem::GetOutputCommunicationDevice()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		Device.Refresh();
		return FAudioDeviceData(Device.CommunicationDevice().Name(), Device.CommunicationDevice().Id());
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return FAudioDeviceData();
}

FAudioDeviceData UVivoxSubSystem::GetInputEffectiveDevice()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		Device.Refresh();
		return FAudioDeviceData(Device.EffectiveDevice().Name(), Device.EffectiveDevice().Id());
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return FAudioDeviceData();
}

FAudioDeviceData UVivoxSubSystem::GetOutputEffectiveDevice()
{
	if (VivoxVoiceClient)
	{
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		Device.Refresh();
		return FAudioDeviceData(Device.EffectiveDevice().Name(), Device.EffectiveDevice().Id());
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return FAudioDeviceData();
}

TMap<FString, FAudioDeviceData> UVivoxSubSystem::GetAvailableInputDevices()
{
	if (VivoxVoiceClient)
	{
		TMap<FString, FAudioDeviceData> AvailableDeviceData;
		auto& Device = VivoxVoiceClient->AudioInputDevices();
		Device.Refresh();

		const TMap<FString, IAudioDevice*>& SourceDevices = Device.AvailableDevices();

		for (const auto& Pair : SourceDevices)
		{
			IAudioDevice* DevicePtr = Pair.Value;
			if (DevicePtr && !DevicePtr->IsEmpty())
			{
				FAudioDeviceData DeviceData(DevicePtr->Name(), DevicePtr->Id());
				AvailableDeviceData.Add(Pair.Key, DeviceData);
			}
		}
		return AvailableDeviceData;
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return TMap<FString, FAudioDeviceData>();
}

TMap<FString, FAudioDeviceData> UVivoxSubSystem::GetAvailableOutputDevices()
{
	if (VivoxVoiceClient)
	{
		TMap<FString, FAudioDeviceData> AvailableDeviceData;
		auto& Device = VivoxVoiceClient->AudioOutputDevices();
		Device.Refresh();

		const TMap<FString, IAudioDevice*>& SourceDevices = Device.AvailableDevices();

		for (const auto& Pair : SourceDevices)
		{
			IAudioDevice* DevicePtr = Pair.Value;
			if (DevicePtr && !DevicePtr->IsEmpty())
			{
				FAudioDeviceData DeviceData(DevicePtr->Name(), DevicePtr->Id());
				AvailableDeviceData.Add(Pair.Key, DeviceData);
			}
		}
		return AvailableDeviceData;
	}
	else
	{
		UE_LOG(LogVivox, Error, TEXT("Vivox is not initialized try initialing it first"));
	}
	return TMap<FString, FAudioDeviceData>();
}

//Transmission functions

bool UVivoxSubSystem::SetTransmissionToNone()
{
	if (LoginSession != nullptr)
	{
		LoginSession->SetTransmissionMode(TransmissionMode::None);
		return true;
	}
	return false;
}

bool UVivoxSubSystem::SetTransmissionToSingleChannel(UVivoxChannelObject* ChannelObject)
{
	if (IsValid(ChannelObject))
	{
		if (LoginSession != nullptr)
		{
			LoginSession->SetTransmissionMode(TransmissionMode::Single, ChannelObject->GetChannel());
			return true;
		}
		return false;
	}
	return false;
}

bool UVivoxSubSystem::SetTransmissionToAll()
{
	if (LoginSession != nullptr)
	{
		LoginSession->SetTransmissionMode(TransmissionMode::All);
		return true;
	}
	return false;
}
