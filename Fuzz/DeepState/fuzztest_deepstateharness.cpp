#include <iostream>
#include <deepstate/DeepState.hpp>
#include "../../STest/Rule/Rule.hpp"
#include "../../STest/Pick/Pick.hpp"

typedef uint32_t UINT32;
using namespace deepstate;

class ArithmeticOperations  
{ 
    public: 
        UINT32 DivisionFunction(UINT32 a, UINT32 b); 
};
UINT32 ArithmeticOperations::DivisionFunction(UINT32 a, UINT32 b) { return (a / b); }

struct Division: public STest::Rule<ArithmeticOperations> 
{
    Division(const char* ruleName);
    bool precondition(const ArithmeticOperations& state);
    void action(ArithmeticOperations& truth);
};

Division::Division(const char* ruleName): STest::Rule<ArithmeticOperations>(ruleName) {}
bool Division::precondition(const ArithmeticOperations &state) { return true; }
void Division::action(ArithmeticOperations &state) 
{
    UINT32 dividend = DeepState_UIntInRange(0, 100), divisor  = DeepState_UIntInRange(0, 100);   
    ASSERT_NE(divisor, 0) << "Divisor cannot be 0! (dividend and divisor values for this run: " << dividend << ", " << divisor << ")";    
    UINT32 result = state.DivisionFunction(dividend, divisor);   
    // Not really concerned about the result if it's a successful case (divisor!=0), but gotta keep the compiler happy with its' unusued variable warning:
    (void)result;
}

TEST(ArithmeticOps, DivisionByZero) 
{
    ArithmeticOperations arithmeticOpsObject;
    Division divisionRuleObject("Division");
    divisionRuleObject.apply(arithmeticOpsObject);
}