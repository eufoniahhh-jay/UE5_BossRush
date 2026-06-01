#include "BossBase.h"
#include "TimerManager.h"
#include "Engine/World.h"

// Sets default values
ABossBase::ABossBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ABossBase::BeginPlay()
{
	Super::BeginPlay();

	ResetBossState();

	/*CurrentHP = MaxHP;
	CurrentPosture = 0.0f;
	bIsDead = false;
	bIsStunned = false;
	OnPostureChanged(CurrentPosture, MaxPosture);*/

	ResetBossState();
}

void ABossBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABossBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABossBase::ApplyBossDamage(float DamageAmount, float PostureDamage)
{
	if (bIsDead)
	{
		return;
	}

	const float SafeDamage = FMath::Max(0.0f, DamageAmount);
	const float SafePostureDamage = FMath::Max(0.0f, PostureDamage);

	float FinalDamage = SafeDamage;

	if (bIsStunned)
	{
		FinalDamage *= StunnedDamageMultiplier;
	}

	CurrentHP = FMath::Clamp(CurrentHP - FinalDamage, 0.0f, MaxHP);

	UE_LOG(LogTemp, Warning, TEXT("[BossBase] Damage %.1f -> HP %.1f / %.1f"),
		FinalDamage, CurrentHP, MaxHP);

	OnBossDamaged(FinalDamage);

	if (CurrentHP <= 0.0f)
	{
		Die();
		return;
	}

	if (SafePostureDamage > 0.0f)
	{
		AddPostureDamage(SafePostureDamage);
	}
}

void ABossBase::AddPostureDamage(float Amount)
{
	if (bIsDead || bIsStunned)
	{
		return;
	}

	const float SafeAmount = FMath::Max(0.0f, Amount);

	if (SafeAmount <= 0.0f)
	{
		return;
	}

	CurrentPosture = FMath::Clamp(CurrentPosture + SafeAmount, 0.0f, MaxPosture);

	UE_LOG(LogTemp, Warning, TEXT("[BossBase] Posture %.1f / %.1f"),
		CurrentPosture, MaxPosture);

	OnPostureChanged(CurrentPosture, MaxPosture);

	if (CurrentPosture >= MaxPosture)
	{
		EnterStun();
	}
}

void ABossBase::EnterStun()
{
	if (bIsDead || bIsStunned)
	{
		return;
	}

	bIsStunned = true;
	CurrentPosture = MaxPosture;

	UE_LOG(LogTemp, Warning, TEXT("[BossBase] Boss Stunned"));

	OnPostureChanged(CurrentPosture, MaxPosture);
	OnBossStunned();

	GetWorldTimerManager().SetTimer(
		StunTimerHandle,
		this,
		&ABossBase::ExitStun,
		StunDuration,
		false
	);
}

void ABossBase::ExitStun()
{
	if (bIsDead)
	{
		return;
	}

	bIsStunned = false;
	CurrentPosture = 0.0f;

	UE_LOG(LogTemp, Warning, TEXT("[BossBase] Boss Stun End"));

	OnPostureChanged(CurrentPosture, MaxPosture);
	OnBossStunEnded();
}

void ABossBase::Die()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsStunned = false;
	CurrentHP = 0.0f;
	CurrentPosture = 0.0f;

	GetWorldTimerManager().ClearTimer(StunTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("[BossBase] Boss Dead"));

	OnPostureChanged(CurrentPosture, MaxPosture);
	OnBossDead();
}

bool ABossBase::IsDead() const
{
	return bIsDead;
}

bool ABossBase::IsStunned() const
{
	return bIsStunned;
}

float ABossBase::GetCurrentHP() const
{
	return CurrentHP;
}

float ABossBase::GetMaxHP() const
{
	return MaxHP;
}

float ABossBase::GetCurrentPosture() const
{
	return CurrentPosture;
}

float ABossBase::GetMaxPosture() const
{
	return MaxPosture;
}

void ABossBase::ResetBossState()
{
	CurrentHP = MaxHP;
	CurrentPosture = 0.0f;

	bIsDead = false;
	bIsStunned = false;

	bCurrentAttackParryable = true;

	UE_LOG(LogTemp, Warning, TEXT("[BossBase] ResetBossState -> HP %.1f / %.1f, Posture %.1f / %.1f"),
		CurrentHP, MaxHP, CurrentPosture, MaxPosture);

	OnPostureChanged(CurrentPosture, MaxPosture);
}

bool ABossBase::IsCurrentAttackParryable() const
{
	return bCurrentAttackParryable;
}