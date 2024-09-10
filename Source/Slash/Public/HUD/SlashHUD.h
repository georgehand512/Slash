// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SlashHUD.generated.h"

class UHUDOverlay;

UCLASS()
class SLASH_API ASlashHUD : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override; 

private:
	UPROPERTY(EditDefaultsOnly, Category = "Slash");
	TSubclassOf<UHUDOverlay> UHUDOverlayClass;

	UPROPERTY();
	UHUDOverlay* HUDOverlay;

public:
	FORCEINLINE UHUDOverlay* GetHUDOverlay() const { return HUDOverlay; }
};

