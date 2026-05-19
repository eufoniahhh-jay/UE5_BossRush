#include "PlayerStatComponent.h"

UPlayerStatComponent::UPlayerStatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStatComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHP = MaxHP;
    CurrentStamina = MaxStamina;
    bIsDead = false;

    UE_LOG(LogTemp, Warning, TEXT("[PlayerStat] Init HP %.1f / %.1f, Stamina %.1f / %.1f"),
        CurrentHP, MaxHP, CurrentStamina, MaxStamina);
}

void UPlayerStatComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsDead && CurrentStamina < MaxStamina)
    {
        RecoverStamina(StaminaRegenPerSecond * DeltaTime);
    }
}

float UPlayerStatComponent::GetHPRatio() const
{
    if (MaxHP <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentHP / MaxHP;
}

float UPlayerStatComponent::GetStaminaRatio() const
{
    if (MaxStamina <= 0.0f)
    {
        return 0.0f;
    }

    return CurrentStamina / MaxStamina;
}

void UPlayerStatComponent::TakeDamage(float DamageAmount)
{
    if (bIsDead)
    {
        return;
    }

    CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);

    UE_LOG(LogTemp, Warning, TEXT("[PlayerStat] TakeDamage %.1f -> HP %.1f / %.1f"),
        DamageAmount, CurrentHP, MaxHP);

    if (CurrentHP <= 0.0f)
    {
        bIsDead = true;
        UE_LOG(LogTemp, Warning, TEXT("[PlayerStat] Player Dead"));
    }
}

void UPlayerStatComponent::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);

    UE_LOG(LogTemp, Warning, TEXT("[PlayerStat] ConsumeStamina %.1f -> Stamina %.1f / %.1f"),
        Amount, CurrentStamina, MaxStamina);
}

void UPlayerStatComponent::RecoverStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);
}

bool UPlayerStatComponent::HasEnoughStamina(float Amount) const
{
    return CurrentStamina >= Amount;
}