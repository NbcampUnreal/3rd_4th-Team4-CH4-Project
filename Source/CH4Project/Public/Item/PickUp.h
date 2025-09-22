#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

class UStaticMeshComponent;

UCLASS()
class CH4PROJECT_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUp();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PickUp")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PickUp")
	TSubclassOf<class UBaseItem> ItemClass;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlap(AActor* OverlapActor, AActor* OtherActor);

};
