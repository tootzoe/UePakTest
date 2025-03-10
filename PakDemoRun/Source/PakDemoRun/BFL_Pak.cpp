// Fill out your copyright notice in the Description page of Project Settings.


#include "BFL_Pak.h"
#include "IPlatformFilePak.h"

bool UBFL_Pak::MountPakFile(const FString& PakFilePath, const FString& PakMountPoint)
{
	int32 PakOrder = 0;
	bool bIsMounted = false;

    // if(GEngine){
    //      GEngine->AddOnScreenDebugMessage(-1, 115.0f, FColor::Yellow, FString::Printf(TEXT("cpp : pakFile for mount: %s") , *PakFilePath));

    // }

    // FString InputPath = PakFilePath;
    // FPaths::MakePlatformFilename(InputPath);



    // if(GEngine){
    //      GEngine->AddOnScreenDebugMessage(-1, 115.0f, FColor::Yellow, FString::Printf(TEXT("InputPath : %s") , *InputPath));

    // }



	//Check to see if running in editor
#if WITH_EDITOR

    UE_LOG(LogTemp, Warning, TEXT("Yes WITH_EDITOR.......vv........."));

    return bIsMounted;

#else


    UE_LOG(LogTemp, Warning, TEXT("Not WITH_EDITOR...........hh.2025-03-10 aaaaaaaa...."));

    if(GEngine){
         GEngine->AddOnScreenDebugMessage(-1, 115.0f, FColor::Yellow, FString::Printf(TEXT("cpp : pakFile for mount: %s") , *PakFilePath));

    }

	FString InputPath = PakFilePath;
	FPaths::MakePlatformFilename(InputPath);

     UE_LOG(LogTemp, Warning, TEXT("  ======== InputPath = %s   "), *(InputPath));


	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));
	if (PakFileMgr == nullptr)
	{
        UE_LOG(LogTemp, Warning, TEXT("Unable to get PakPlatformFile for pak file (mount): %s !!!!!!!!11   "), *(PakFilePath));

		return false;
    }

	if (!PakMountPoint.IsEmpty())
	{
        UE_LOG(LogTemp, Warning, TEXT("PakMountPoint NOT ........ IsEmpty...."));

		bIsMounted = PakFileMgr->Mount(*InputPath, PakOrder, *PakMountPoint);
	}
	else
	{
        UE_LOG(LogTemp, Warning, TEXT("PakMountPoint is ........ IsEmpty...."));
		bIsMounted = PakFileMgr->Mount(*InputPath, PakOrder);
	}

    UE_LOG(LogTemp, Warning, TEXT("PakFileMgr->Mount result: %d .... ") , bIsMounted);

	return bIsMounted;

 #endif
}

bool UBFL_Pak::UnmountPakFile(const FString& PakFilePath)
{
	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")));


	if (PakFileMgr == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Unable to get PakPlatformFile for pak file (Unmount): %s"), *(PakFilePath));
		return false;
	}

	return PakFileMgr->Unmount(*PakFilePath);
}

void UBFL_Pak::RegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{

    UE_LOG(LogTemp, Warning, TEXT("RegisterMountPoint ......."));


	FPackageName::RegisterMountPoint(RootPath, ContentPath);
}

void UBFL_Pak::UnRegisterMountPoint(const FString& RootPath, const FString& ContentPath)
{
	FPackageName::UnRegisterMountPoint(RootPath, ContentPath);
}

FString const UBFL_Pak::GetPakMountPoint(const FString& PakFilePath)
{
	FPakFile* Pak = nullptr;
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")), *PakFilePath, false);
	Pak = PakFile.GetReference();
	if (Pak->IsValid())
	{
		return Pak->GetMountPoint();
	}
	return FString();
}

TArray<FString> UBFL_Pak::GetPakContent(const FString& PakFilePath, bool bOnlyCooked /*= true*/)
{
	FPakFile* Pak = nullptr;
	TRefCountPtr<FPakFile> PakFile = new FPakFile(FPlatformFileManager::Get().FindPlatformFile(TEXT("PakFile")), *PakFilePath, false);
	Pak = PakFile.GetReference();
	TArray<FString> PakContent;

	if (Pak->IsValid())
	{
		FString ContentPath, PakAppendPath;
		FString MountPoint = GetPakMountPoint(PakFilePath);
		MountPoint.Split("/Content/", &ContentPath, &PakAppendPath);


		TArray<FPakFile::FFilenameIterator> Records;
		for (FPakFile::FFilenameIterator It(*Pak, false); It; ++It)
		{
			Records.Add(It);
		}

		for (auto& It : Records)
		{
			if (bOnlyCooked)
			{
				if (FPaths::GetExtension(It.Filename()) == TEXT("uasset"))
				{
					PakContent.Add(FString::Printf(TEXT("%s%s"), *PakAppendPath, *It.Filename()));
				}
			}
			else
			{
				PakContent.Add(FString::Printf(TEXT("%s%s"), *PakAppendPath, *It.Filename()));
			}
		}
	}
	return PakContent;
}

FString UBFL_Pak::GetPakMountContentPath(const FString& PakFilePath)
{
	FString ContentPath, RightStr;
	bool bIsContent;
	FString MountPoint = GetPakMountPoint(PakFilePath);
	bIsContent = MountPoint.Split("/Content/", &ContentPath, &RightStr);
	if (bIsContent)
	{
		return FString::Printf(TEXT("%s/Content/"), *ContentPath);
	}
	// Check Pak Content for /Content/ Path
	else
	{
		TArray<FString> Content = GetPakContent(PakFilePath);
		if (Content.Num() > 0)
		{
			FString FullPath = FString::Printf(TEXT("%s%s"), *MountPoint, *Content[0]);
			bIsContent = FullPath.Split("/Content/", &ContentPath, &RightStr);
			if (bIsContent)
			{
				return FString::Printf(TEXT("%s/Content/"), *ContentPath);
			}
		}
	}
	//Failed to Find Content
	return FString("");
}

void UBFL_Pak::MountAndRegisterPak(FString PakFilePath, bool& bIsMountSuccessful)
{
	FString PakRootPath = "/Game/";
	if (!PakFilePath.IsEmpty())
	{
		if (MountPakFile(PakFilePath, FString()))
		{
			bIsMountSuccessful = true;
			const FString MountPoint = GetPakMountContentPath(PakFilePath);
			RegisterMountPoint(PakRootPath, MountPoint);
		}
	}
}

UClass* UBFL_Pak::LoadPakObjClassReference(FString PakContentPath)
{
	FString PakRootPath = "/Game/";
	const FString FileName = Conv_PakContentPathToReferenceString(PakContentPath, PakRootPath);

    UE_LOG(LogTemp, Warning, TEXT("------------Conv_  class name: %s") , *FileName);

	return LoadPakFileClass(FileName);
}

UClass* UBFL_Pak::LoadPakFileClass(const FString& FileName)
{
    //const FString Name = FileName + TEXT(".") + FPackageName::GetShortName(FileName) + TEXT("_C");
    const FString Name = FileName   + TEXT("_C");

     UE_LOG(LogTemp, Warning, TEXT("------------ complete class name: %s") , *Name);

	return StaticLoadClass(UObject::StaticClass(), nullptr, *Name);
}

FString UBFL_Pak::Conv_PakContentPathToReferenceString(const FString PakContentPath, const FString PakMountPath)
{
	return FString::Printf(TEXT("%s%s.%s"),
		*PakMountPath, *FPaths::GetBaseFilename(PakContentPath, false), *FPaths::GetBaseFilename(PakContentPath, true));
}
