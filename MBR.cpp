#include "MBR.h"

MBR::Partition::Partition(size_t sector_count, MBR::Partition::Status s, MBR::Partition::Type t)
    : m_status(s)
    , m_type(t)
    , m_sector_count(static_cast<uint32_t>(sector_count))
{
}

void MBR::Partition::serialize(uint8_t* into, const DiskGeometry& geometry, size_t lba_offset) const
{
    if (m_type == Partition::Type::FAT32_CHS || geometry.within_chs_limit())
    {
        auto chs_begin = to_chs(lba_offset, geometry);
        auto chs_end = to_chs(lba_offset + m_sector_count, geometry);

        if (chs_begin.head > ((1 << 8) - 1) || chs_end.head > ((1 << 8) - 1))
            throw std::runtime_error("head number cannot exceed 2^7");
        if (chs_begin.sector > ((1 << 6) - 1) || chs_end.sector > ((1 << 6) - 1))
            throw std::runtime_error("sector number cannot exceed 2^5");
        if (chs_begin.cylinder > ((1 << 10) - 1) || chs_end.cylinder > ((1 << 10) - 1))
            throw std::runtime_error("sector number cannot exceed 2^9");

        // CC SSSSSS
        // 00 000000
        uint8_t sector_cylinder_begin = static_cast<uint8_t>(chs_begin.sector);
        uint8_t sector_cylinder_end = static_cast<uint8_t>(chs_end.sector);

        sector_cylinder_begin |= chs_begin.cylinder & 0b1100000000;
        sector_cylinder_end   |= chs_end.cylinder   & 0b1100000000;

        uint8_t cylinder_begin = chs_begin.cylinder & 0b0011111111;
        uint8_t cylinder_end   = chs_end.cylinder   & 0b0011111111;

        into[1] = static_cast<uint8_t>(chs_begin.head);
        into[2] = sector_cylinder_begin;
        into[3] = cylinder_begin;
        into[5] = static_cast<uint8_t>(chs_end.head);
        into[6] = sector_cylinder_end;
        into[7] = cylinder_end;
    }
    else if (m_type == Partition::Type::FAT32_LBA)
    {
        into[1] = 0xFF;
        into[2] = 0xFF;
        into[3] = 0xFF;
        into[5] = 0xFF;
        into[6] = 0xFF;
        into[7] = 0xFF;
    }