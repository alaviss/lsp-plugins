/*
 * dyna_processor.cpp
 *
 *  Created on: 21 окт. 2016 г.
 *      Author: sadko
 */

#ifndef LSP_NO_EXPERIMENTAL

#include <core/types.h>
#include <metadata/plugins.h>
#include <metadata/ports.h>
#include <metadata/developers.h>

namespace lsp
{
    //-------------------------------------------------------------------------
    // Graphic Equalizer
    static const int dyna_processor_classes[] =
    {
        C_DYNAMICS, C_COMPRESSOR, C_ENVELOPE, C_EXPANDER, C_GATE, C_LIMITER, -1
    };

    static const char *dyna_proc_sc_modes[] =
    {
        "Peak",
        "RMS",
        "Low-Pass",
        "Uniform",
        NULL
    };

    static const char *dyna_proc_sc_sources[] =
    {
        "Middle",
        "Side",
        "Left",
        "Right",
        NULL
    };

    static const char *dyna_proc_sc_type[] =
    {
        "Feed-forward",
        "Feed-back",
        NULL
    };

    static const char *dyna_proc_extsc_type[] =
    {
        "Feed-forward",
        "Feed-back",
        "External",
        NULL
    };

    #define DYNA_PROC_COMMON     \
        BYPASS,             \
        IN_GAIN,            \
        OUT_GAIN,           \
        SWITCH("pause", "Pause graph analysis", 0.0f), \
        TRIGGER("clear", "Clear graph analysis")

    #define DYNA_PROC_MS_COMMON  \
        DYNA_PROC_COMMON,        \
        SWITCH("msl", "Mid/Side listen", 0.0f)

    #define DYNA_PROC_SC_MONO_CHANNEL(sct) \
        COMBO("sct", "Sidechain type", dyna_processor_base_metadata::SC_TYPE_DFL, sct), \
        COMBO("scm", "Sidechain mode", dyna_processor_base_metadata::SC_MODE_DFL, dyna_proc_sc_modes), \
        SWITCH("scl", "Sidechain listen", 0.0f), \
        LOG_CONTROL("scr", "Sidechain reactivity", U_MSEC, dyna_processor_base_metadata::REACTIVITY), \
        AMP_GAIN100("scp", "Sidechain preamp", GAIN_AMP_0_DB)

    #define DYNA_PROC_SC_STEREO_CHANNEL(id, label, sct) \
        COMBO("sct" id, "Sidechain type" label, dyna_processor_base_metadata::SC_TYPE_DFL, sct), \
        COMBO("scm" id, "Sidechain mode" label, dyna_processor_base_metadata::SC_MODE_DFL, dyna_proc_sc_modes), \
        SWITCH("scl" id, "Sidechain listen" label, 0.0f), \
        COMBO("scs" id, "Sidechain source" label, dyna_processor_base_metadata::SC_SOURCE_DFL, dyna_proc_sc_sources), \
        LOG_CONTROL("scr" id, "Sidechain reactivity" label, U_MSEC, dyna_processor_base_metadata::REACTIVITY), \
        AMP_GAIN100("scp" id, "Sidechain preamp" label, GAIN_AMP_0_DB)

    #define DYNA_POINT(idx, id, label, level) \
        SWITCH("pe" #idx id, "Point enable " #idx label, 0.0f), \
        LOG_CONTROL_DFL("tl" #idx id, "Threshold " #idx label, U_GAIN_AMP, dyna_processor_base_metadata::THRESHOLD, level), \
        LOG_CONTROL_DFL("gl" #idx id, "Gain " #idx label, U_GAIN_AMP, dyna_processor_base_metadata::THRESHOLD, level), \
        LOG_CONTROL("kn" #idx id, "Knee " #idx label, U_GAIN_AMP, dyna_processor_base_metadata::KNEE), \
        SWITCH("ae" #idx id, "Attack enable " #idx label, 0.0f), \
        LOG_CONTROL_DFL("al" #idx id, "Attack level " #idx label, U_GAIN_AMP, dyna_processor_base_metadata::ATTACK_LVL, level), \
        LOG_CONTROL("at" #idx id, "Attack time " #idx label, U_MSEC, dyna_processor_base_metadata::ATTACK_TIME), \
        SWITCH("re" #idx id, "Release enable " #idx label, 0.0f), \
        LOG_CONTROL_DFL("rl" #idx id, "Relative level " #idx label, U_GAIN_AMP, dyna_processor_base_metadata::RELEASE_LVL, level), \
        LOG_CONTROL("rt" #idx id, "Release time " #idx label, U_MSEC, dyna_processor_base_metadata::RELEASE_TIME)

    #define DYNA_PROC_CHANNEL(id, label) \
        LOG_CONTROL("atd" id, "Attack time default" label, U_MSEC, dyna_processor_base_metadata::ATTACK_TIME), \
        LOG_CONTROL("rtd" id, "Release time default" label, U_MSEC, dyna_processor_base_metadata::RELEASE_TIME), \
        DYNA_POINT(0, id, label, GAIN_AMP_M_48_DB), \
        DYNA_POINT(1, id, label, GAIN_AMP_M_36_DB), \
        DYNA_POINT(2, id, label, GAIN_AMP_M_24_DB), \
        DYNA_POINT(3, id, label, GAIN_AMP_M_12_DB), \
        LOG_CONTROL("llr" id, "Low-level ratio" label, U_NONE, dyna_processor_base_metadata::RATIO), \
        LOG_CONTROL("hlr" id, "High-level ratio" label, U_NONE, dyna_processor_base_metadata::RATIO), \
        LOG_CONTROL("omk" id, "Overall makeup gain" label, U_GAIN_AMP, dyna_processor_base_metadata::MAKEUP), \
        AMP_GAIN10("cdr" id, "Dry gain" label, GAIN_AMP_M_INF_DB),     \
        AMP_GAIN10("cwt" id, "Wet gain" label, GAIN_AMP_0_DB), \
        SWITCH("slv" id, "Sidechain level visibility" label, 1.0f), \
        SWITCH("elv" id, "Envelope level visibility" label, 1.0f), \
        SWITCH("grv" id, "Gain reduction visibility" label, 1.0f), \
        MESH("cmg" id, "Curve modelling graph" label, 2, dyna_processor_base_metadata::CURVE_MESH_SIZE), \
        MESH("ccg" id, "Curve graph" label, 2, dyna_processor_base_metadata::CURVE_MESH_SIZE), \
        MESH("scg" id, "Sidechain graph" label, 2, dyna_processor_base_metadata::TIME_MESH_SIZE), \
        MESH("evg" id, "Envelope graph" label, 2, dyna_processor_base_metadata::TIME_MESH_SIZE), \
        MESH("grg" id, "Gain reduciton" label, 2, dyna_processor_base_metadata::TIME_MESH_SIZE), \
        METER_OUT_GAIN("slm" id, "Sidechain level meter" label, GAIN_AMP_P_24_DB), \
        METER_OUT_GAIN("clm" id, "Curve level meter" label, GAIN_AMP_P_24_DB), \
        METER_OUT_GAIN("elm" id, "Envelope level meter" label, GAIN_AMP_P_24_DB), \
        METER_GAIN("rlm" id, "Reduction level meter" label, GAIN_AMP_P_24_DB)

    #define DYNA_PROC_AUDIO_METER(id, label) \
        SWITCH("ilv" id, "Input level visibility" label, 1.0f), \
        SWITCH("olv" id, "Output level visibility" label, 1.0f), \
        MESH("isg" id, "Input signal graph" label, 2, dyna_processor_base_metadata::TIME_MESH_SIZE), \
        MESH("osg" id, "Output signal graph" label, 2, dyna_processor_base_metadata::TIME_MESH_SIZE), \
        METER_GAIN("ilm" id, "Input level meter" label, GAIN_AMP_P_24_DB), \
        METER_GAIN("olm" id, "Output level meter" label, GAIN_AMP_P_24_DB)

    static const port_t dyna_processor_mono_ports[] =
    {
        PORTS_MONO_PLUGIN,
        DYNA_PROC_COMMON,
        DYNA_PROC_SC_MONO_CHANNEL(dyna_proc_sc_type),
        DYNA_PROC_CHANNEL("", ""),
        DYNA_PROC_AUDIO_METER("", ""),

        PORTS_END
    };

    static const port_t dyna_processor_stereo_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        DYNA_PROC_COMMON,
        DYNA_PROC_SC_STEREO_CHANNEL("", "", dyna_proc_sc_type),
        DYNA_PROC_CHANNEL("", ""),
        DYNA_PROC_AUDIO_METER("_l", " Left"),
        DYNA_PROC_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t dyna_processor_lr_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        DYNA_PROC_COMMON,
        DYNA_PROC_SC_STEREO_CHANNEL("_l", " Left", dyna_proc_sc_type),
        DYNA_PROC_SC_STEREO_CHANNEL("_r", " Right", dyna_proc_sc_type),
        DYNA_PROC_CHANNEL("_l", " Left"),
        DYNA_PROC_CHANNEL("_r", " Right"),
        DYNA_PROC_AUDIO_METER("_l", " Left"),
        DYNA_PROC_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t dyna_processor_ms_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        DYNA_PROC_MS_COMMON,
        DYNA_PROC_SC_STEREO_CHANNEL("_m", " Mid", dyna_proc_sc_type),
        DYNA_PROC_SC_STEREO_CHANNEL("_s", " Side", dyna_proc_sc_type),
        DYNA_PROC_CHANNEL("_m", " Mid"),
        DYNA_PROC_CHANNEL("_s", " Side"),
        DYNA_PROC_AUDIO_METER("_m", " Mid"),
        DYNA_PROC_AUDIO_METER("_s", " Side"),

        PORTS_END
    };

    static const port_t sc_dyna_processor_mono_ports[] =
    {
        PORTS_MONO_PLUGIN,
        PORTS_MONO_SIDECHAIN,
        DYNA_PROC_COMMON,
        DYNA_PROC_SC_MONO_CHANNEL(dyna_proc_extsc_type),
        DYNA_PROC_CHANNEL("", ""),
        DYNA_PROC_AUDIO_METER("", ""),

        PORTS_END
    };

    static const port_t sc_dyna_processor_stereo_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        PORTS_STEREO_SIDECHAIN,
        DYNA_PROC_COMMON,
        DYNA_PROC_SC_STEREO_CHANNEL("", "", dyna_proc_extsc_type),
        DYNA_PROC_CHANNEL("", ""),
        DYNA_PROC_AUDIO_METER("_l", " Left"),
        DYNA_PROC_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t sc_dyna_processor_lr_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        PORTS_STEREO_SIDECHAIN,
        DYNA_PROC_COMMON,
        DYNA_PROC_SC_STEREO_CHANNEL("_l", " Left", dyna_proc_extsc_type),
        DYNA_PROC_SC_STEREO_CHANNEL("_r", " Right", dyna_proc_extsc_type),
        DYNA_PROC_CHANNEL("_l", " Left"),
        DYNA_PROC_CHANNEL("_r", " Right"),
        DYNA_PROC_AUDIO_METER("_l", " Left"),
        DYNA_PROC_AUDIO_METER("_r", " Right"),

        PORTS_END
    };

    static const port_t sc_dyna_processor_ms_ports[] =
    {
        PORTS_STEREO_PLUGIN,
        PORTS_STEREO_SIDECHAIN,
        DYNA_PROC_MS_COMMON,
        DYNA_PROC_SC_STEREO_CHANNEL("_m", " Mid", dyna_proc_extsc_type),
        DYNA_PROC_SC_STEREO_CHANNEL("_s", " Side", dyna_proc_extsc_type),
        DYNA_PROC_CHANNEL("_m", " Mid"),
        DYNA_PROC_CHANNEL("_s", " Side"),
        DYNA_PROC_AUDIO_METER("_m", " Mid"),
        DYNA_PROC_AUDIO_METER("_s", " Side"),

        PORTS_END
    };

    // Dynamic Processor
    const plugin_metadata_t  dyna_processor_mono_metadata::metadata =
    {
        "Dynamischer Prozessor Mono",
        "Dynamic Processor Mono",
        "DP1M",
        &developers::v_sadovnikov,
        "dyna_processor_mono",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 0,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        dyna_processor_mono_ports,
        NULL
    };

    const plugin_metadata_t  dyna_processor_stereo_metadata::metadata =
    {
        "Dynamischer Prozessor Stereo",
        "Dynamic Processor Stereo",
        "DP1S",
        &developers::v_sadovnikov,
        "dyna_processor_stereo",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 1,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        dyna_processor_stereo_ports,
        stereo_plugin_port_groups
    };

    const plugin_metadata_t  dyna_processor_lr_metadata::metadata =
    {
        "Dynamischer Prozessor LeftRight",
        "Dynamic Processor LeftRight",
        "DP1LR",
        &developers::v_sadovnikov,
        "dyna_processor_lr",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 2,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        dyna_processor_lr_ports,
        stereo_plugin_port_groups
    };

    const plugin_metadata_t  dyna_processor_ms_metadata::metadata =
    {
        "Dynamischer Prozessor MidSide",
        "Dynamic Processor MidSide",
        "DP1MS",
        &developers::v_sadovnikov,
        "dyna_processor_ms",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 3,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        dyna_processor_ms_ports,
        stereo_plugin_port_groups
    };

    // Sidechain compressor
    const plugin_metadata_t  sc_dyna_processor_mono_metadata::metadata =
    {
        "Dynamischer Sidechain-Prozessor Mono",
        "Sidechain Dynamic Processor Mono",
        "DSCP1M",
        &developers::v_sadovnikov,
        "sc_dyna_processor_mono",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 4,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        sc_dyna_processor_mono_ports,
        mono_plugin_sidechain_port_groups
    };

    const plugin_metadata_t  sc_dyna_processor_stereo_metadata::metadata =
    {
        "Dynamischer Sidechain-Prozessor Stereo",
        "Sidechain Dynamic Processor Stereo",
        "DSCP1S",
        &developers::v_sadovnikov,
        "sc_dyna_processor_stereo",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 5,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        sc_dyna_processor_stereo_ports,
        stereo_plugin_sidechain_port_groups
    };

    const plugin_metadata_t  sc_dyna_processor_lr_metadata::metadata =
    {
        "Dynamischer Sidechain-Prozessor LeftRight",
        "Sidechain Dynamic Processor LeftRight",
        "DSCP1LR",
        &developers::v_sadovnikov,
        "sc_dyna_processor_lr",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 6,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        sc_dyna_processor_lr_ports,
        stereo_plugin_sidechain_port_groups
    };

    const plugin_metadata_t  sc_dyna_processor_ms_metadata::metadata =
    {
        "Dynamischer Sidechain-Prozessor MidSide",
        "Sidechain Dynamic Processor MidSide",
        "DSCP1MS",
        &developers::v_sadovnikov,
        "sc_dyna_processor_ms",
        "----",
        LSP_DYNAMIC_PROCESSOR_BASE + 7,
        LSP_VERSION(1, 0, 0),
        dyna_processor_classes,
        sc_dyna_processor_ms_ports,
        stereo_plugin_sidechain_port_groups
    };
}

#endif