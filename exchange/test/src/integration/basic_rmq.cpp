#include "exchange/algos/dev_mode/dev_mode.hpp"
#include "exchange/process_spawning/spawning.hpp"
#include "exchange/rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
#include "test_utils/process.hpp"

#include <gtest/gtest.h>

namespace rmq = nutc::rabbitmq;

class IntegrationBasic : public ::testing::Test {
protected:
    void
    SetUp() override
    {
        auto& rmq_conn = rmq::RabbitMQConnectionManager::get_instance();

        if (!rmq_conn.connected_to_rabbitmq()) {
            FAIL() << "Failed to connect to rabbitmq";
        }
    }

    void
    TearDown() override
    {
        nutc::testing_utils::kill_all_processes(users_);
        rmq::RabbitMQConnectionManager::reset_instance();
    }

    nutc::manager::ClientManager users_; // NOLINT (*)
};

TEST_F(IntegrationBasic, InitialLiquidity)
{
    nutc::algo_mgmt::DevModeAlgoManager algo_manager =
        nutc::algo_mgmt::DevModeAlgoManager(1);
    algo_manager.initialize_files();
    algo_manager.initialize_client_manager(users_);
    nutc::client::spawn_all_clients(users_);
}
