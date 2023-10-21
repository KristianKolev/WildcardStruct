// Example Blueprint function that receives any struct as input
UFUNCTION(BlueprintCallable, Category = "Example", CustomThunk, meta = (CustomStructureParam = "AnyStruct"))
static void ReceiveSomeStruct(UProperty* AnyStruct);
 
DECLARE_FUNCTION(execReceiveSomeStruct)
{
    // Steps into the stack, walking to the next property in it
    Stack.Step(Stack.Object, NULL);
 
    // Grab the last property found when we walked the stack
    // This does not contains the property value, only its type information
    UStructProperty* StructProperty = ExactCast<UStructProperty>(Stack.MostRecentProperty);
 
    // Grab the base address where the struct actually stores its data
    // This is where the property value is truly stored
    void* StructPtr = Stack.MostRecentPropertyAddress;
 
    // We need this to wrap up the stack
    P_FINISH;
 
    // Iterate through the struct
    IterateThroughStructProperty(StructProperty, StructPtr);
}
 
/* Example function for parsing a single property
* @param Property    the property reflection data
* @param ValuePtr    the pointer to the property value
*/
void ParseProperty(UProperty* Property, void* ValuePtr)
{        
        
    float FloatValue;
    int32 IntValue;
    bool BoolValue;
    FString StringValue;
    FName NameValue;
    FText TextValue;
 
 
    // Here's how to read integer and float properties
    if (UNumericProperty *NumericProperty = Cast<UNumericProperty>(Property))
    {
        if (NumericProperty->IsFloatingPoint())
        {
            FloatValue = NumericProperty->GetFloatingPointPropertyValue(ValuePtr);
        }
        else if (NumericProperty->IsInteger())
        {
            IntValue = NumericProperty->GetSignedIntPropertyValue(ValuePtr);
        }
    }
 
    // How to read booleans
    if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
    {
        BoolValue = BoolProperty->GetPropertyValue(ValuePtr);
    }
 
    // Reading names
    if (UNameProperty* NameProperty = Cast<UNameProperty>(Property))
    {
        NameValue = NameProperty->GetPropertyValue(ValuePtr);
    }
 
    // Reading strings
    if (UStrProperty* StringProperty = Cast<UStrProperty>(Property))
    {
        StringValue = StringProperty->GetPropertyValue(ValuePtr);
    }
 
    // Reading texts
    if (UTextProperty* TextProperty = Cast<UTextProperty>(Property))
    {
        TextValue = TextProperty->GetPropertyValue(ValuePtr);
    }
 
    // Reading an array
    if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property))
    {
        // We need the helper to get to the items of the array            
        FScriptArrayHelper Helper(ArrayProperty, ValuePtr);
        for (int32 i = 0, n = Helper.Num(); i<n; ++i)
        {    
            ParseProperty(ArrayProperty->Inner, Helper.GetRawPtr(i));
        }
    }
 
    // Reading a nested struct
    if (UStructProperty* StructProperty = Cast<UStructProperty>(Property))
    {
        IterateThroughStructProperty(StructProperty, ValuePtr);
    }
}
 
/*
* Example function for iterating through all properties of a struct
* @param StructProperty    The struct property reflection data
* @param StructPtr        The pointer to the struct value
*/
void IterateThroughStructProperty(UStructProperty* StructProperty, void* StructPtr)
{
    // Walk the structs' properties
    UScriptStruct* Struct = StructProperty->Struct;
    for (TFieldIterator<UProperty> It(Struct); It; ++It)
    {
        UProperty* Property = *It;
 
        // This is the variable name if you need it
        FString VariableName = Property->GetName();
 
        // Never assume ArrayDim is always 1
        for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ArrayIndex++)
        {
            // This grabs the pointer to where the property value is stored
            void* ValuePtr = Property->ContainerPtrToValuePtr<void>(StructPtr, ArrayIndex);
 
            // Parse this property
            ParseProperty(Property, ValuePtr);
        }
    }
}
