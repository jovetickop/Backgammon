#pragma once

// 领域层：包含所有DDD核心概念。
// - 值对象：不可变的领域概念
// - 聚合根：领域对象的根
// - 领域服务：处理业务逻辑

#include "values/values.h"
#include "aggregates/aggregates.h"
#include "services/services.h"
