#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrapActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class CH4PROJECT_API ATrapActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrapActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trap")
	UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trap")
	UStaticMeshComponent* TrapMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap")
	float StunDuration = 3.f;  

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlap(AActor* OverlappedActor, AActor* OtherActor);

};
