// AsirikuyStrategiesWrapper.cpp
// C API wrapper bridging legacy C exports to C++ strategy implementation
// Maintains backward compatibility with MQL4/MQL5 code

#include "StrategyContext.hpp"
#include "StrategyFactory.hpp"
#include "IStrategy.hpp"
#include "AsirikuyDefines.h"
#include "StrategyTypes.h"

extern "C" {

/**
 * @brief Main strategy execution entry point (C API)
 * @details Called from MQL code. Routes to appropriate C++ strategy via factory.
 * 
 * @param params Pointer to StrategyParams structure with all execution context
 * @return AsirikuyReturnCode indicating success/failure
 * 
 * @note This function provides exception safety boundary between C++ and C code
 */
AsirikuyReturnCode runStrategy(StrategyParams* params) {
    // Validate input
    if (params == NULL) {
        return NULL_POINTER;
    }
    
    try {
        // Create C++ context wrapper
        trading::StrategyContext context(params);
        
        // Get strategy ID from context
        StrategyId strategyId = context.getStrategyId();
        
        // Create strategy instance from factory
        trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
        trading::IStrategy* strategy = factory.createStrategy(strategyId);
        
        if (strategy == NULL) {
            // Strategy not registered in factory
            return INVALID_STRATEGY;
        }
        
        // Validate strategy can execute with this context
        if (!strategy->validate(context)) {
            delete strategy;
            return INVALID_PARAMETER;
        }
        
        // Execute strategy
        AsirikuyReturnCode result = strategy->execute(context);
        
        // Clean up
        delete strategy;
        
        return result;
        
    } catch (const std::bad_alloc&) {
        // Memory allocation failure
        return INSUFFICIENT_MEMORY;
        
    } catch (const std::out_of_range&) {
        // Array access out of bounds
        return INVALID_PARAMETER;
        
    } catch (const std::exception&) {
        // Any other standard exception
        return TA_LIB_ERROR;  // Generic error code
        
    } catch (...) {
        // Non-standard exception
        return TA_LIB_ERROR;  // Generic error code
    }
}

/**
 * @brief Get strategy name by ID (C API)
 * @details Utility function for debugging and logging
 * 
 * @param strategyId Strategy identifier
 * @return Pointer to static string with strategy name, or NULL if not found
 * 
 * @note Returned pointer is valid until next call to this function
 */
const char* getStrategyName(StrategyId strategyId) {
    static char nameBuffer[256];
    
    try {
        trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
        
        if (!factory.isRegistered(strategyId)) {
            return NULL;
        }
        
        trading::IStrategy* strategy = factory.createStrategy(strategyId);
        if (strategy == NULL) {
            return NULL;
        }
        
        std::string name = strategy->getName();
        
        // Copy to static buffer (thread-unsafe but compatible with C API)
        if (name.length() >= sizeof(nameBuffer)) {
            name = name.substr(0, sizeof(nameBuffer) - 1);
        }
        
        // Use safe string copy
        #ifdef _MSC_VER
            strcpy_s(nameBuffer, sizeof(nameBuffer), name.c_str());
        #else
            std::strncpy(nameBuffer, name.c_str(), sizeof(nameBuffer) - 1);
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';
        #endif
        
        delete strategy;
        
        return nameBuffer;
        
    } catch (...) {
        return NULL;
    }
}

/**
 * @brief Check if strategy is registered (C API)
 * @details Allows MQL code to validate strategy ID before calling runStrategy
 * 
 * @param strategyId Strategy identifier to check
 * @return 1 if registered, 0 otherwise
 */
int isStrategyRegistered(StrategyId strategyId) {
    try {
        trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
        return factory.isRegistered(strategyId) ? 1 : 0;
        
    } catch (...) {
        return 0;
    }
}

/**
 * @brief Get count of registered strategies (C API)
 * @details Diagnostic function for system health checks
 * 
 * @return Number of registered strategies, or 0 on error
 */
int getRegisteredStrategyCount() {
    try {
        trading::StrategyFactory& factory = trading::StrategyFactory::getInstance();
        return static_cast<int>(factory.getRegisteredCount());
        
    } catch (...) {
        return 0;
    }
}

} // extern "C"
