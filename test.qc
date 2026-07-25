loc version_code_major 26;
loc version_code_minor 1;
loc version_code_patch 4;
loc build_profile "release";
loc target_arch "x86_64";
loc optimization_level "O3";
loc api_version "v2";
loc server_domain "api.libqcc.org";
loc db_cluster "cluster-prod-01";
loc db_port "5432";
loc log_root "/var/log/libqcc";
loc diagnostic_mode "verbose";
loc cache_eviction_strategy "LRU";

project {
  name "LibQCC Core Framework";
  version "@loc(version_code_major).@loc(version_code_minor).@loc(version_code_patch)";
  version_code "@loc(version_code_major)";
  environment "@loc(build_profile)";
};

compiler_settings {
  target "@loc(target_arch)";
  flags "-Wall -Wextra -Werror -std=c++20";
  optimize "@loc(optimization_level)";
  inline_functions "true";
  strict_aliasing "false";
};

network {
  protocol "https";
  host "@loc(server_domain)";
  base_path "/@loc(api_version)/gateway";
  timeout_ms "5000";
  max_retry_attempts "3";
  security {
    ssl_enabled "true";
    min_tls_version "1.3";
    cipher_suite "ECDHE-RSA-AES256-GCM-SHA384";
  };
};

database {
  engine "postgresql";
  cluster_id "@loc(db_cluster)";
  port "@loc(db_port)";
  connection_timeout "30";
  pooling {
    min_idle "5";
    max_active "50";
    max_lifetime_sec "1800";
  };
  sharding {
    enabled "true";
    keyspaces "user_id, transaction_id";
  };
};

logging {
  destination "@loc(log_root)/app.log";
  level "@loc(diagnostic_mode)";
  format "json";
  rotate_size_mb "100";
  max_backup_files "10";
  telemetry {
    prometheus_enabled "true";
    metrics_port "9090";
    scrape_interval_sec "15";
  };
};

memory_cache {
  max_memory_bytes "2147483648";
  eviction "@loc(cache_eviction_strategy)";
  ttl_seconds "3600";
  threading {
    worker_threads "8";
    io_threads "4";
    priority "high";
  };
};