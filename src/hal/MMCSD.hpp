#ifndef HAL_MMCSD_HPP
#define HAL_MMCSD_HPP

#include <cstdint>

namespace HAL::MMCSD
{
	struct MMCSDContext_t
	{
		uint32_t capa;
		uint32_t systemConfig;
		uint32_t ctrlInfo;
		uint32_t hctl;
		uint32_t sysCtl;
		uint32_t pState;
	};

	void construct_mmcsd(void * base_addr) noexcept;
	int  soft_reset() noexcept;
	void  system_config(uint32_t config) noexcept;
	uint32_t  is_card_inserted() noexcept;
	uint32_t  is_hsupported() noexcept;
	uint32_t  is_card_write_protected() noexcept;
	int  lines_reset(uint32_t flag) noexcept;
	void  supported_volt_set(uint32_t volt) noexcept;

	uint32_t  is_int_clock_stable(uint32_t retry) noexcept;
		int  int_clock(unsigned int pwr) noexcept;

	int  bus_freq_set(uint32_t freq_in, uint32_t freq_out, uint32_t bypass) noexcept;
	void  bus_width_set(uint32_t width) noexcept;
	int  bus_power(uint32_t pwr) noexcept;
	void  bus_volt_set(uint32_t volt) noexcept;

	void  command_send(uint32_t cmd, uint32_t cmdarg, const void *data, uint32_t nblks, uint32_t dmaEn) noexcept;
	void  data_get(unsigned char *data, unsigned int len) noexcept;
	uint32_t  is_xfer_complete(uint32_t retry) noexcept;
	uint32_t  is_cmd_complete(uint32_t retry) noexcept;
	void  response_get(uint32_t *rsp) noexcept;
	void  blk_len_set(uint32_t blklen) noexcept;
	int  init_stream_send() noexcept;
	void  data_timeout_set(uint32_t timeout) noexcept;

    uint32_t  intr_status_get(uint32_t flag) noexcept;
    void  intr_enable(uint32_t flag) noexcept;
	void  intr_status_enable(uint32_t flag) noexcept;
    void  intr_status_disable(uint32_t flag) noexcept;
    void  intr_status_clear(uint32_t flag) noexcept;

    void  context_save(MMCSDContext_t *contextPtr) noexcept;
	void  context_restore(const MMCSDContext_t *contextPtr) noexcept;

}
#endif //HAL_MMCSD_HPP