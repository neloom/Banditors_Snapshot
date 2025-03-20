// Copyright (C) 2024 by Haguk Kim


#include "PlayerNameTagWidget.h"

void UPlayerNameTagWidget::SetName(FString Name)
{
	NameTagTextBox->SetText(FText::FromString(Name));
}
