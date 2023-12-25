#include "local_algos/dev_mode.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/connection_manager/RabbitMQConnectionManager.hpp"
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
    bool algo_success = nutc::dev_mode::create_mt_algo_files(1);
    EXPECT_TRUE(algo_success);
    size_t num_clients = nutc::client::initialize(users_, Mode::DEV, 1);
    EXPECT_EQ(num_clients, 1);
}
