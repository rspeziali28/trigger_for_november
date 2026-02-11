#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>   // std::min, std::max
#include <cstdint>     // uint64_t
#include <atomic>      // std::memory_order
#include <TFile.h>
#include <TTree.h>
#include <TString.h>    



struct WaveformRecord {
    uint8_t  TriggerIDFPGA;
    uint16_t TriggerIDSRSRaw;
    uint64_t timestampRaw;   // contiene il valore a 40 bit
};


std::vector<WaveformRecord> read_waveform_file(const std::string& filename);

void ReadBinTrigger_forNovember() {

    std::string filename = "/home/riccardo-speziali/Scrivania/bin_file/sampic_run222.bin";
    auto records = read_waveform_file(filename);

    std::cout << "Read " << records.size() << " waveform record(s)\n";

    TString outfile_name = "output.root";
    TFile outfile(outfile_name,"RECREATE");

    TTree *sampic_tree = new TTree("picoTree", "SAMPIC output in ROOT format");

    WaveformRecord rec;

    sampic_tree->Branch("TriggerIDFPGA", &rec.TriggerIDFPGA, "TriggerIDFPGA/b");
    sampic_tree->Branch("TriggerIDSRSRaw", &rec.TriggerIDSRSRaw, "TriggerIDSRSRaw/s");
    sampic_tree->Branch("timestampRaw", &rec.timestampRaw, "timestampRaw/l");

    for (const auto& r : records) {
        rec = r;
        sampic_tree->Fill();
    }

    sampic_tree->Write();
    outfile.Close();
}




std::vector<WaveformRecord> read_waveform_file(const std::string& filename) {

    std::vector<WaveformRecord> records;
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return records;
    }

    while (true) {
        WaveformRecord record;

        // Leggi TriggerIDFPGA (1 byte)
        file.read(reinterpret_cast<char*>(&record.TriggerIDFPGA), sizeof(uint8_t));

        // Leggi TriggerIDSRSRaw (2 byte)
        file.read(reinterpret_cast<char*>(&record.TriggerIDSRSRaw), sizeof(uint16_t));

        // Leggi timestamp a 40 bit (5 byte)
        uint8_t buffer[5];
        file.read(reinterpret_cast<char*>(buffer), 5);

        if (!file) break;

        record.timestampRaw =
              (uint64_t)buffer[0]
            | ((uint64_t)buffer[1] << 8)
            | ((uint64_t)buffer[2] << 16)
            | ((uint64_t)buffer[3] << 24)
            | ((uint64_t)buffer[4] << 32);

        records.push_back(record);
    }

    return records;
}
