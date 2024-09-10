// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/HUDOverlay.h"


void ASlashHUD::BeginPlay()
{
	Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        APlayerController* Controller = World->GetFirstPlayerController();
        if (Controller && UHUDOverlayClass)
        {
            HUDOverlay = CreateWidget<UHUDOverlay>(Controller, UHUDOverlayClass);
            HUDOverlay->AddToViewport();
        }
    }

    
}
