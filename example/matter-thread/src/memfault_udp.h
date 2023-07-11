/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#pragma once

void memfault_init_udp_message(void);
void init_memfault_chunks_sender(void);

int memfault_server_connect(void);
int memfault_server_init(void);

void memfault_schedule();