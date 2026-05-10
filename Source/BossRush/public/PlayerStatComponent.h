#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOSSRUSH_API UPlayerStatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerStatComponent();

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetCurrentHP() const { return CurrentHP; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetMaxHP() const { return MaxHP; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetCurrentStamina() const { return CurrentStamina; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetMaxStamina() const { return MaxStamina; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetHPRatio() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetStaminaRatio() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RecoverStamina(float Amount);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|HP", meta = (AllowPrivateAccess = "true"))
    float MaxHP = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|HP", meta = (AllowPrivateAccess = "true"))
    float CurrentHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina", meta = (AllowPrivateAccess = "true"))
    float MaxStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Stamina", meta = (AllowPrivateAccess = "true"))
    float CurrentStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;
};