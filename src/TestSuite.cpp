#include "TestSuite.h"
#include <map> 

std::vector<uint8_t> TestSuite::getLdRegD8Test() {
    return {
        0x3E, 0xAA,       
        0x06, 0xBB,       
        0x0E, 0xCC,       
        0x16, 0xDD,       
        0x1E, 0xEE,       
        0x26, 0xF0,       
        0x2E, 0x0F,       
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdRegPairD16Test() {
    return {
        0x01, 0x34, 0x12, 
        0x11, 0x78, 0x56, 
        0x21, 0xBC, 0x9A, 
        0x31, 0xFE, 0xFF, 
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdRegRegTest() {
    return {
        0x3E, 0x11,       
        0x06, 0x22,       
        0x47,             
        0x0E, 0x33,       
        0x79,             
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdRegMhlTest() {
    
    
    return {
        0x21, 0x80, 0xFF, 
        0x3E, 0xAB,       
        0x77,             
        0x06, 0x00,       
        0x46,             
        0x3E, 0x00,       
        0x7E,             
        0x76              
    };
}


std::vector<uint8_t> TestSuite::getLdAMrrTest() {
    
    return {
        0x21, 0x80, 0xFF, 
        0x3E, 0xCC,       
        0x77,             
        0x21, 0x82, 0xFF, 
        0x3E, 0xDD,       
        0x77,             

        0x01, 0x80, 0xFF, 
        0x11, 0x82, 0xFF, 
        0x3E, 0x00,       
        0x0A,             
        
        0x1A,             
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdMrrATest() {
    
    return {
        0x01, 0x8A, 0xFF, 
        0x11, 0x8C, 0xFF, 
        0x3E, 0xEE,       
        0x02,             
        0x3E, 0xFF,       
        0x12,             
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdAMa16Test() {
    
    return {
        0x21, 0x90, 0xFF, 
        0x3E, 0xBA,       
        0x77,             
        0x3E, 0x00,       
        0xFA, 0x90, 0xFF, 
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdMa16ATest() {
    return {
        0x3E, 0xCD,       
        0xEA, 0x95, 0xFF, 
        0x76              
    };
}


std::vector<uint8_t> TestSuite::getLdMhlRegTest() { 
    return {
        0x21, 0x80, 0xFF, 
        0x06, 0xAA,       
        0x70,             
        0x3E, 0xBB,       
        0x77,             
        0x76              
    };
}
std::vector<uint8_t> TestSuite::getLdMhlD8Test() {
    return {
        0x21, 0x85, 0xFF, 
        0x36, 0xCC,       
        0x76              
    };
}


std::vector<uint8_t> TestSuite::getIncRegNoWrapTest() {
    return {
        0x06, 0x10, 0x04, 
        0x0E, 0x20, 0x0C, 
        0xAF,       0x3C, 
        0x76
    };
}
std::vector<uint8_t> TestSuite::getIncRegWrapHalfCarryTest() {
    return {
        0x3E, 0x0F, 0x3C, 
        0x06, 0xFF, 0x04, 
        0x76
    };
}
std::vector<uint8_t> TestSuite::getDecRegsTest() {
    return {
        0x0E, 0x11, 0x0D, 
        0x0E, 0x10, 0x0D, 
        0x3E, 0x00, 0x3D, 
        0x76
    };
}
std::vector<uint8_t> TestSuite::getIncMhlTest() {
    return {
        0x21, 0x80, 0xFF, 
        0x3E, 0xFE, 0x77, 
        0x34,             
        0x34,             
        0x76
    };
}


std::vector<uint8_t> TestSuite::getAddARegNoCarryTest() {
    return { 0x3E, 0x12, 0x06, 0x03, 0x80, 0x76 }; 
}
std::vector<uint8_t> TestSuite::getAddARegWithHalfCarryTest() {
    return { 0x3E, 0x0F, 0x06, 0x01, 0x80, 0x76 }; 
}
std::vector<uint8_t> TestSuite::getAddARegWithCarryTest() {
    return { 0x3E, 0xF0, 0x06, 0x11, 0x80, 0x76 }; 
}
std::vector<uint8_t> TestSuite::getSubARegNoBorrowTest() {
    return { 0x3E, 0x15, 0x06, 0x03, 0x90, 0x76 }; 
}
std::vector<uint8_t> TestSuite::getSubARegWithHalfBorrowTest() {
    return { 0x3E, 0x10, 0x06, 0x01, 0x90, 0x76 }; 
}
std::vector<uint8_t> TestSuite::getSubARegWithBorrowTest() {
    return { 0x3E, 0x10, 0x06, 0x20, 0x90, 0x76 }; 
}
std::vector<uint8_t> TestSuite::getXorATest() {
    return { 0x3E, 0x55, 0xAF, 0x76 }; 
}



TestSuite::TestSuite() {
    populateTests();
}

const std::vector<TestRom>& TestSuite::getAllTests() const {
    return available_tests_;
}

const TestRom* TestSuite::getTestByName(const std::string& name) const {
    for (const auto& test : available_tests_) {
        if (test.name == name) {
            return &test;
        }
    }
    return nullptr;
}

void TestSuite::populateTests() {
    available_tests_.clear();

    
    available_tests_.push_back({ "LD r, d8",           getLdRegD8Test(), 0x0000 });
    available_tests_.push_back({ "LD rr, d16",         getLdRegPairD16Test(), 0x0000 });
    available_tests_.push_back({ "LD r, r'",           getLdRegRegTest(), 0x0000 });
    available_tests_.push_back({ "LD r, (HL)",         getLdRegMhlTest(), 0x0000 });

    
    available_tests_.push_back({ "LD A, (BC/DE)",      getLdAMrrTest(), 0x0000 });
    available_tests_.push_back({ "LD (BC/DE), A",      getLdMrrATest(), 0x0000 });
    available_tests_.push_back({ "LD A, (nn)",         getLdAMa16Test(), 0x0000 });
    available_tests_.push_back({ "LD (nn), A",         getLdMa16ATest(), 0x0000 });

    
    available_tests_.push_back({ "LD (HL), r",         getLdMhlRegTest(), 0x0000 });
    available_tests_.push_back({ "LD (HL), d8",        getLdMhlD8Test(), 0x0000 });

    
    available_tests_.push_back({ "INC r (No Wrap)",    getIncRegNoWrapTest(), 0x0000 });
    available_tests_.push_back({ "INC r (Wrap/HC)",    getIncRegWrapHalfCarryTest(), 0x0000 });
    available_tests_.push_back({ "DEC r (Flags)",      getDecRegsTest(), 0x0000 });
    available_tests_.push_back({ "INC (HL)",           getIncMhlTest(), 0x0000 });

    
    available_tests_.push_back({ "ADD A,r (No Cy)",    getAddARegNoCarryTest(), 0x0000 });
    available_tests_.push_back({ "ADD A,r (HC)",       getAddARegWithHalfCarryTest(), 0x0000 });
    available_tests_.push_back({ "ADD A,r (FC)",       getAddARegWithCarryTest(), 0x0000 });
    available_tests_.push_back({ "SUB A,r (No Bw)",    getSubARegNoBorrowTest(), 0x0000 });
    available_tests_.push_back({ "SUB A,r (HB)",       getSubARegWithHalfBorrowTest(), 0x0000 });
    available_tests_.push_back({ "SUB A,r (FB)",       getSubARegWithBorrowTest(), 0x0000 });
    available_tests_.push_back({ "XOR A,A",            getXorATest(), 0x0000 });
}