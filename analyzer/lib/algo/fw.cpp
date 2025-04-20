#include "fw.h"

using namespace std;

arro::algo::BinFWTable::BinFWTable(BinFWTable&& other) : _sz(other._sz), _buf(other._buf) {
	other._sz = 0;
	other._buf = nullptr;
}

arro::algo::BinFWTable::~BinFWTable() {
	if (_buf) delete[] _buf;
}

arro::algo::BinFWTable::Row arro::algo::BinFWTable::operator[](size_t idx) const {
	if (idx >= _sz) throw out_of_range("Row index out of FW table bounds");

	return Row(this, idx);
}

const double& arro::algo::BinFWTable::Row::operator[](size_t idx) const {
	if (idx >= _table->_sz) throw out_of_range("Column index out of FW table bounds");

	return _table->_buf[_idx * _table->_sz + idx];
}

void arro::algo::BinFWTable::binDumpToFile(const string& path, const vector<vector<double>>& table) {
	int fd = open(path.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd == -1) throw invalid_argument("Unable to acquire FW table file '" + path + "'");
	size_t sz = table.size();

	write(fd, "\x58\x45", 2);
	write(fd, &sz, sizeof(size_t));

	for (auto row : table) {
		for (auto elem : row) {
			write(fd, &elem, sizeof(double));
		}
	}

	close(fd);
}

arro::algo::BinFWTable arro::algo::BinFWTable::readFromBinFile(const string& path) {
	int fd = open(path.c_str(), O_RDONLY);

	if (fd == -1) throw invalid_argument("Unable to acquire FW table file '" + path + "'");

	unsigned char buf[2];
	int bytesRead = read(fd, buf, 2);

	if (bytesRead != 2 || buf[0] != 0x58 || buf[1] != 0x45) throw invalid_argument("Failed to read magic bytes");

	size_t sz;
	bytesRead = read(fd, &sz, sizeof(size_t));
	if (bytesRead != sizeof(size_t)) throw invalid_argument("Failed to read size of table");

	double* table = new double[sz];
	bytesRead = read(fd, table, sizeof(double) * sz);
	if (bytesRead != sizeof(double) * sz) throw invalid_argument("Failed to read table memory");

	return BinFWTable(table, sz);
}