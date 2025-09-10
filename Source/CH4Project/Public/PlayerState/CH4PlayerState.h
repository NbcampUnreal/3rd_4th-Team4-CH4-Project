// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Type/MatchTypes.h"
#include "CH4PlayerState.generated.h"

/**
 * - 멀티플레이어에서 플레이어 역할 관리 및 체포 횟수 관리
 * - PlayerRole은 Replication + RepNotify
 * - 체포 횟수 CurrentArrests/MaxArrests는 Replication
 * - 서버 권한으로 역할 변경, 클라이언트에게 RPC로 알림 가능
 */
UCLASS()
class CH4PROJECT_API ACH4PlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	ACH4PlayerState();

	/** 플레이어 역할 */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly)
	EPlayerRole PlayerRole;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentArrests, BlueprintReadOnly)
	int32 CurrentArrests;

	/** 최대 체포 횟수 */
	UPROPERTY(ReplicatedUsing = OnRep_MaxArrests, BlueprintReadOnly)
	int32 MaxArrests;

	void SetCurrentArrests(int32 NewCurrentArrests);
	void SetMaxArrests(int32 NewMaxArrests);
	
	UFUNCTION()
	void OnRep_CurrentArrests();

	UFUNCTION()
	void OnRep_MaxArrests();

	
	/** 역할 설정 (서버 권한 확인 후 RPC 호출 가능) */
	void SetPlayerRole(EPlayerRole NewRole);

	/** 역할 조회 */
	EPlayerRole GetPlayerRole() const { return PlayerRole; }

	/** 서버 RPC: 권한 없는 클라이언트에서 호출 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetPlayerRole(EPlayerRole NewRole);
	void ServerSetPlayerRole_Implementation(EPlayerRole NewRole);
	bool ServerSetPlayerRole_Validate(EPlayerRole NewRole);

	/** 역할 변경 RepNotify */
	UFUNCTION()
	void OnRep_PlayerRole();

	/** 클라이언트 RPC: 서버에서 역할 배정 직후 즉시 알림 */
	UFUNCTION(Client, Reliable)
	void ClientReceiveRole(EPlayerRole NewRole);
	void ClientReceiveRole_Implementation(EPlayerRole NewRole);

protected:
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

};
