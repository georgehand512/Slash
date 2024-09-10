// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HealthBarComponent.h"
#include "HUD/HealthBar.h"
#include "Components/ProgressBar.h"

void UHealthBarComponent::SetHealthPercent(float Percent)
{
	// only perform CAST on first call, as casting is expensive if done too often
    if (HealthBarWidget == nullptr)
    {
        HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());
    } 

    if (HealthBarWidget && HealthBarWidget->HealthBar)
    {
        HealthBarWidget->HealthBar->SetPercent(Percent);
    }
}