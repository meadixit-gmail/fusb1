/*******************************************************************************
 * @file     dpm_util.h
 * @author   USB PD Firmware Team
 *
 * @copyright @parblock
 * Copyright &copy; 2020 ON Semiconductor &reg;. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at [ONSMI_T&C.pdf]
 * (http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf)
 *
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 * @endparblock
 ******************************************************************************/
#include "pd_types.h"
#include "dev_tcpd.h"
#include "dpm.h"

unsigned fw_version_check(uint16_t *current_ver, uint16_t new_ver_0, uint16_t new_ver_1,
                          uint16_t new_ver_2, uint16_t new_ver_3);

void enable_swd(void *dpm_priv, bool en);

bool dpm_vbus_valid(void *dpm_priv, uint32_t id, uint32_t low, uint32_t high);
void dpm_vbus_disch(void *dpm_priv, uint32_t id, VBUS_DISCH_T disch);
