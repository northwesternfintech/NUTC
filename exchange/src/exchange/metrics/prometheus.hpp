#pragma once

#undef DEBUG

#include <prometheus/exposer.h>
#include <prometheus/registry.h>

namespace nutc::exchange {

namespace ps = prometheus;

class Prometheus {
    std::unique_ptr<prometheus::Exposer> exposer;
    std::shared_ptr<ps::Registry> registry = std::make_shared<ps::Registry>();

    Prometheus()
    {
        const auto* env_var = std::getenv("NUTC_EXPOSE_METRICS");
        if (env_var && std::string(env_var) == "1") {
            exposer = std::make_unique<prometheus::Exposer>("0.0.0.0:4152");
            exposer->RegisterCollectable(registry);
        }
    }

    static Prometheus&
    get()
    {
        static Prometheus instance{};
        return instance;
    }

public:
    static const std::shared_ptr<ps::Registry>
    get_registry()
    {
        return get().registry;
    }
};

} // namespace nutc::exchange
