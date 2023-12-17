#include "local_algos/dev_mode.hpp"
#include "process_spawning/spawning.hpp"
#include "rabbitmq/rabbitmq.hpp"
#include "test_utils/process.hpp"

#include <gtest/gtest.h>

namespace rmq = nutc::rabbitmq;

class IntegrationBasic : public ::testing::Test {
protected:
    // TODO: teardown of rmq connection if necessary
    void
    SetUp() override
    {
        auto& rmq_conn = rmq::RabbitMQConnectionManager::getInstance();

        if (!rmq_conn.connectedToRMQ()) {
            log_e(rabbitmq, "Failed to initialize connection");
            exit(1);
        }
    }

    void
    TearDown() override
    {
        nutc::testing_utils::kill_all_processes(users);
        rmq::RabbitMQConnectionManager::resetInstance();
    }

    nutc::manager::ClientManager users;
};

TEST_F(IntegrationBasic, InitialLiquidity)
{
    nutc::dev_mode::create_mt_algo_files(1);
    size_t num_clients = nutc::client::initialize(users, Mode::DEV, 1);
    EXPECT_EQ(num_clients, 1);
}
