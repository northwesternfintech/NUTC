#pragma once
#include "exchange/matching/engine/engine.hpp"

#include <optional>
#include <string>

using engine_ref_t = std::reference_wrapper<nutc::matching::Engine>;

namespace nutc {
/**
 * @brief Manages all matching engines for arbitrary tickers
 */
namespace engine_manager {
/**
 * @class Manager
 * @brief Manages all matching engines for arbitrary tickers
 * @details This class is responsible for creating and managing all matching engines for
 * different tickers
 */
class EngineManager {
public:
    /**
     * @brief Returns a reference to the engine with the given ticker, if it exists
     * @param ticker The ticker of the engine to return
     * @return A reference to the engine with the given ticker, if it exists
     */
    std::optional<engine_ref_t> get_engine(const std::string& ticker);

    /**
     * @brief Adds an engine with the given ticker
     * @param ticker The ticker of the engine to add
     * @return A reference to the engine with the given ticker
     */
    void add_engine(const std::string& ticker);

    /** @brief Adds initial liquidity by creating fake sell orders for a given ticker at
     * a given quantity/price
     */
    void add_initial_liquidity(const std::string& ticker, float quantity, float price);

private:
    std::map<std::string, matching::Engine> engines_;
    EngineManager() = default;

public:
    // fuck it, everything's a singleton

    static EngineManager&
    get_instance()
    {
        static EngineManager instance;
        return instance;
    }

    EngineManager(EngineManager const&) = delete;
    EngineManager operator=(EngineManager const&) = delete;
    EngineManager(EngineManager&&) = delete;
    EngineManager operator=(EngineManager&&) = delete;
};
} // namespace engine_manager
} // namespace nutc
