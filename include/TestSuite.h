#ifndef TEST_SUITE_H
#define TEST_SUITE_H

#include <vector>
#include <string>
#include <cstdint>

struct TestRom {
    std::string name;
    std::vector<uint8_t> data;
    uint16_t initial_pc = 0x0000;
};

class TestSuite {
public:
    TestSuite();

    const std::vector<TestRom>& getAllTests() const;
    const TestRom* getTestByName(const std::string& name) const;

private:
    std::vector<TestRom> available_tests_;
    void populateTests();
    
    static std::vector<uint8_t> getLdRegD8Test();         
    static std::vector<uint8_t> getLdRegPairD16Test();    
    static std::vector<uint8_t> getLdRegRegTest();        
    static std::vector<uint8_t> getLdRegMhlTest();        
    
    static std::vector<uint8_t> getLdAMrrTest();          
    static std::vector<uint8_t> getLdMrrATest();          
    static std::vector<uint8_t> getLdAMa16Test();         
    static std::vector<uint8_t> getLdMa16ATest();         
    
    static std::vector<uint8_t> getLdMhlRegTest();        
    static std::vector<uint8_t> getLdMhlD8Test();         
    
    static std::vector<uint8_t> getIncRegNoWrapTest();
    static std::vector<uint8_t> getIncRegWrapHalfCarryTest();
    static std::vector<uint8_t> getDecRegsTest();
    static std::vector<uint8_t> getIncMhlTest();
    
    static std::vector<uint8_t> getAddARegNoCarryTest();
    static std::vector<uint8_t> getAddARegWithHalfCarryTest();
    static std::vector<uint8_t> getAddARegWithCarryTest();
    static std::vector<uint8_t> getSubARegNoBorrowTest();
    static std::vector<uint8_t> getSubARegWithHalfBorrowTest();
    static std::vector<uint8_t> getSubARegWithBorrowTest();
    static std::vector<uint8_t> getXorATest(); 
};

#endif 