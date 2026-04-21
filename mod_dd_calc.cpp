#include <Arduino.h>
#include "mod_dd_calc.h"
#include "mod_eeprom.h"
#include "mod_sim900a.h"

float    g_accumulated_dd = 0.0f;
uint32_t g_day_count      = 0;

static float    s_temp_sum       = 0.0f;
static uint32_t s_sample_cnt     = 0;
static bool     s_alert_sent     = false;

// millis() 溢出安全计时：记录上次结算时的 millis() 值
static uint32_t s_day_start_ms   = 0;

// -------------------------------------------------------
// millis() 溢出安全的经过时间计算
// -------------------------------------------------------
static uint32_t elapsedMs(uint32_t start) {
    return millis() - start;  // unsigned 减法自动处理溢出
}

// -------------------------------------------------------
// 初始化
// -------------------------------------------------------
void DD_Init(float saved_dd) {
    g_accumulated_dd = saved_dd;
    s_temp_sum       = 0.0f;
    s_sample_cnt     = 0;
    s_alert_sent     = false;
    s_day_start_ms   = millis();

    // 从 EEPROM 恢复已结算天数（用于日志显示）
    uint32_t saved_days = EEPROM_ReadUint32(ADDR_LAST_SETTLE_DAY);
    g_day_count = (saved_days < 10000) ? saved_days : 0; // 合法性检查

    Serial.print("[DD] 初始化完成，累积DD=");
    Serial.print(g_accumulated_dd, 1);
    Serial.print("  历史结算天数=");
    Serial.println(g_day_count);
    Serial.println("[DD] 下次结算在开机后 24 小时");
}

// -------------------------------------------------------
// 每次采样调用
// -------------------------------------------------------
void DD_Update(float temp) {
    s_temp_sum += temp;
    s_sample_cnt++;

    // 判断是否满 24 小时
    if (elapsedMs(s_day_start_ms) >= DAY_MS) {
        s_day_start_ms = millis();  // 重置计时起点

        if (s_sample_cnt == 0) return;

        float daily_mean = s_temp_sum / (float)s_sample_cnt;
        float daily_dd   = daily_mean - DD_BASE_TEMP;
        if (daily_dd < 0.0f) daily_dd = 0.0f;

        g_accumulated_dd += daily_dd;
        g_day_count++;

        s_temp_sum   = 0.0f;
        s_sample_cnt = 0;

        // 保存到 EEPROM
        EEPROM_SaveDD(g_accumulated_dd);
        EEPROM_WriteUint32(ADDR_LAST_SETTLE_DAY, g_day_count);

        Serial.println("[DD] ====== 24h 日结算 ======");
        Serial.print("[DD] 第 "); Serial.print(g_day_count);
        Serial.print(" 天  均温="); Serial.print(daily_mean, 1);
        Serial.print("C  今日DD="); Serial.print(daily_dd, 1);
        Serial.print("  累积DD="); Serial.println(g_accumulated_dd, 1);

        // 积温告警（只发一次）
        if (g_accumulated_dd >= DD_ALERT_THRESH && !s_alert_sent) {
            Serial.println("[DD] ⚠️ 积温超过阈值，发送告警短信");
            SIM900A_SendDDAlert(g_accumulated_dd);
            s_alert_sent = true;
        }
    }
}

// -------------------------------------------------------
// 工具函数
// -------------------------------------------------------
float DD_GetAccumulated() {
    return g_accumulated_dd;
}

void DD_ResetDaily() {
    g_accumulated_dd = 0.0f;
    s_alert_sent     = false;
    s_temp_sum       = 0.0f;
    s_sample_cnt     = 0;
    s_day_start_ms   = millis();
    g_day_count      = 0;
    EEPROM_SaveDD(0.0f);
    EEPROM_WriteUint32(ADDR_LAST_SETTLE_DAY, 0);
    Serial.println("[DD] 积温已手动重置为0");
}

void DD_PrintStatus() {
    uint32_t elapsed = elapsedMs(s_day_start_ms);
    uint32_t remain  = DAY_MS - elapsed;
    uint32_t remain_h = remain / 3600000UL;
    uint32_t remain_m = (remain % 3600000UL) / 60000UL;

    Serial.print("[DD] 第 "); Serial.print(g_day_count + 1);
    Serial.print(" 天运行中  累积DD="); Serial.print(g_accumulated_dd, 1);
    Serial.print("  距下次结算: ");
    Serial.print(remain_h); Serial.print("h ");
    Serial.print(remain_m); Serial.println("m");
}