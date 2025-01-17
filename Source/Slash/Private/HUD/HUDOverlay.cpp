// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HUDOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHUDOverlay::SetHealthBarPercent(float Percent)
{
	if (PlayerHealthBar)
    {
        PlayerHealthBar->SetPercent(Percent);
    }
}

void UHUDOverlay::SetStaminaBarPercent(float Percent)
{
	if (PlayerStaminaBar)
    {
        PlayerStaminaBar->SetPercent(Percent);
    }	
}

void UHUDOverlay::SetGold(int32 Gold)
{
	if (GoldText)
    {
        GoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Gold)));
    }
}

void UHUDOverlay::SetSouls(int32 Souls)
{
	if (SoulsText)
    {
        SoulsText->SetText(FText::FromString(FString::Printf(TEXT("%d"), Souls)));
    }	
}
