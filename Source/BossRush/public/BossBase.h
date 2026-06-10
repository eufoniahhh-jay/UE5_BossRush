// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossBase.generated.h"

UCLASS()
class BOSSRUSH_API ABossBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABossBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	FTimerHandle StunTimerHandle;

public:
	// Day12.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	float MaxHP = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	float CurrentHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Posture")
	float MaxPosture = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Posture")
	float CurrentPosture = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Posture")
	float StunDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Stats")
	float StunnedDamageMultiplier = 1.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool bIsStunned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Attack")
	bool bCurrentAttackParryable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|State")
	bool bIsInvincible = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Boss|Damage")
	virtual void ApplyBossDamage(float DamageAmount, float PostureDamage);

	UFUNCTION(BlueprintCallable, Category = "Boss|Posture")
	virtual void AddPostureDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	virtual void EnterStun();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	virtual void ExitStun();

	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	virtual void Die();

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Boss|State")
	bool IsStunned() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Stats")
	float GetCurrentHP() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Stats")
	float GetMaxHP() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Posture")
	float GetCurrentPosture() const;

	UFUNCTION(BlueprintPure, Category = "Boss|Posture")
	float GetMaxPosture() const;


	// Day20. 보스 초기화 함수
	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	void ResetBossState();

	UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
	bool IsCurrentAttackParryable() const;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Event")
	void OnBossDamaged(float FinalDamage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Event")
	void OnPostureChanged(float NewPosture, float MaxValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Event")
	void OnBossStunned();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Event")
	void OnBossStunEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Event")
	void OnBossDead();
};
