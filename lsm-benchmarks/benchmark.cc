#include <cassert>
#include <limits.h>
#include <cstdio>
#include <iostream>
#include <random>
#include <string>
#include <unistd.h>

#include "leveldb/db.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/table.h"

using random_bytes_engine = std::independent_bits_engine<
	std::default_random_engine, CHAR_BIT, unsigned char>;
static const char *db_name = "/mnt/db/leveldb";
size_t SEQ_WRITES = 100000000;
size_t  num_queries = 10000000;
int FLAGS_key_size = 128;
int FLAGS_value_size = 512;


void print_line_header(int table_size) {
	std::cout << table_size << ", " << SEQ_WRITES << ", " << SEQ_WRITES*(FLAGS_key_size + FLAGS_value_size)/1024.0/1024.0;
}

void print_result(double time_elapsed) {
	double micros_op = (time_elapsed * 1.0E6) / num_queries;
	double throughput = (FLAGS_value_size + FLAGS_key_size) * num_queries / ( 1.0E6 * time_elapsed); 
	std::cout << "," << time_elapsed << "," << micros_op;
	std::cout << "," << throughput;
}

void parse_result(int flag) {

    char buff[100];
    //system("umount -l /dev/sdb4");
    system("kill $(pidof -s blktrace)");
    switch (flag) {
        case 0 :
            sprintf(buff, "blkparse -a issue -f \"%%n\\\\n\" -i leveldb.seq.tracefile -o leveldb.seq.txt");
            break;

        case 1 :
            sprintf(buff, "blkparse -a issue -f \"%%n\\\\n\" -i leveldb.rand.tracefile -o leveldb.rand.txt");
            break;

        case 2 :
            sprintf(buff, "blkparse -a issue -f \"%%n\\\\n\" -i rocksdb.seq.tracefile -o rocksdb.seq.txt");
            break;

        case 3 :
            sprintf(buff, "blkparse -a issue -f \"%%n\\\\n\" -i rocksdb.rand.tracefile -o rocksdb.rand.txt");
            break;

        default: 
            break;
    }

    system(buff); 

    FILE * fp;
    switch (flag) { 
        case 0 :
            fp = fopen("leveldb.seq.txt", "r");
            break;

        case 1 :
            fp = fopen("leveldb.rand.txt", "r");
            break;

        case 2 :
            fp = fopen("rocksdb.seq.txt", "r");
            break;

        case 3 :
            fp = fopen("rocksdb.rand.txt", "r");
            break;

        default : 
            break;
    }
    // FILE * fp = fopen("parsefile.txt", "r");
    int temp, ret, total = 0;

    ret = fscanf(fp, "%d", &temp);
    while (ret && temp) {
        total += temp;
        ret = fscanf(fp, "%d", &temp);
    }

    fclose(fp);
    std::cout << ", " << total << ", " << total*512.0/((FLAGS_key_size + FLAGS_value_size)*SEQ_WRITES);
    //system("mount /dev/sdb4 /mnt/db/leveldb");

}

std::string create_key(int key_num, char letter) {
	char key[100];
	snprintf(key, sizeof(key), "%016d%c", key_num, letter);
	std::string cpp_key = key;
	cpp_key.insert(cpp_key.begin(), FLAGS_key_size - 17, ' ');
	return cpp_key;  
}

int run_leveldb(int table_size) {
	/***************************SETUP********************************/
	/* Setup and open database */
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	options.error_if_exists = true;
	options.max_file_size = table_size;
	options.write_buffer_size = table_size;
	options.compression = leveldb::CompressionType::kNoCompression;
	//options.block_size = table_size;
	leveldb::Status status = leveldb::DB::Open(options, db_name, &db);
	if (!status.ok()) std::cerr << status.ToString() << std::endl;
	assert(status.ok());

	/* Create random number generator and initialize variables */
	random_bytes_engine rbe;
	std::vector<unsigned char> data(FLAGS_value_size);


        /* Setup DB with SEQ_WRITES * (FLAGS_value_size + FLAGS_data_size) amount of data */
        for (size_t i = 0; i < SEQ_WRITES; i++) {
                std::generate(begin(data), end(data), std::ref(rbe));
                std::string key = create_key(i, 'a');
                std::string value (data.begin(), data.end());
                leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
                if (!s.ok()) {
                        std::cout << "Assertion failed" << std::endl;
                        std::cerr << s.ToString() << std::endl;
                        return 1;
                }
        }

        sync();

        /****************PREP BUFFERS WITH RANDOM INSERTS*****************/
        srand(12321);
        std::string value;
        for (size_t i = 0; i < num_queries; i++) {
                std::string key = create_key(rand() % SEQ_WRITES, 'b');
                leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
                assert(s.ok());
        }
        sync();


        system("blktrace -a write -d /dev/sdb4 -o leveldb.seq.tracefile &");

	/****************TEST SEQUENTIAL INSERTS*************************/
	for (size_t i = 0; i < SEQ_WRITES; i++) {
		std::generate(begin(data), end(data), std::ref(rbe));
		std::string key = create_key(i, 'a');
		std::string value (data.begin(), data.end());
		leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
		if (!s.ok()) {
			std::cout << "Assertion failed" << std::endl;
			std::cerr << s.ToString() << std::endl;
			return 1;
		}
	}

        sync();
        parse_result(0);

        /*****************PREP BUFFERS WITH RANDOM INSERTES**************/
        srand(12321);
        for (size_t i = 0; i < num_queries; i++) {
            std::string key = create_key(rand() % SEQ_WRITES, 'b');
            leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
            assert(s.ok());
        }
        sync();


	/*****************TEST RANDOM INSERTS****************************/

        srand(32123); 
        system("blktrace -a write -d /dev/sdb4 -o leveldb.rand.tracefile &");

	for(size_t i = 0; i < num_queries; i++) {
		std::string key = create_key(rand() % SEQ_WRITES, 'b');
		leveldb::Status s = db->Put(leveldb::WriteOptions(), key, value);
		assert(s.ok());
	}
        
        sync();
        parse_result(1);


	/****************************CLEANUP*****************************/
	delete db;
	return 0;

}

int run_rocksdb(int table_size) {
	/***************************SETUP********************************/
	/* Setup and open database */
	rocksdb::DB* db;
	rocksdb::Options options;
	options.create_if_missing = true;
	options.error_if_exists = true;
	//rocksdb::BlockBasedTableOptions table_options;
	//table_options.metadata_block_size = table_size;
	//table_options.block_size = table_size;
	//options.table_factory.reset(rocksdb::NewBlockBasedTableFactory(table_options));
	options.target_file_size_base = table_size;
	options.write_buffer_size = table_size;
	options.db_write_buffer_size = table_size;
	options.max_open_files = 1000;
	options.compression = rocksdb::CompressionType::kNoCompression;
	rocksdb::Status status = rocksdb::DB::Open(options, db_name, &db);
	if (!status.ok()) {
		std::cerr << status.ToString() << std::endl;
	}
	assert(status.ok());

	/* Create random number generator and initialize variables */
	random_bytes_engine rbe;
	std::vector<unsigned char> data(FLAGS_value_size);

        /* Setup DB with SEQ_WRITES * (FLACS_value_size + FLAGS_key_size) abmount of data */
        for (size_t i = 0; i < SEQ_WRITES; i++) {
                std::generate(begin(data), end(data), std::ref(rbe));
                std::string key = create_key(i, 'a');
                std::string value (data.begin(), data.end());
                rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
                if (!s.ok()) {
                        std::cout << "Assertion failed" << std::endl;
                        std::cout << s.ToString() << std::endl;
                        return 1;
                }
        }
        sync();

        /****************PREP BUFFERS WITH RANDOM INSERTS****************/
        srand(12321);
        std::string value;
        for (size_t i = 0; i < num_queries; i++) {
                std::string key = create_key(rand() % SEQ_WRITES, 'b');
                rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
                if (!s.ok()) std::cout << s.ToString() << std::endl;
                assert(s.ok());
        }
        sync();


        system("blktrace -a write -d /dev/sdb4 -o rocksdb.seq.tracefile &");

	/***************TEST SEQUENTIAL WRITES**********************/
	for (size_t i = 0; i < SEQ_WRITES; i++) {
		std::generate(begin(data), end(data), std::ref(rbe));
		std::string key = create_key(i, 'a');
		std::string value (data.begin(), data.end());
		rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
		if (!s.ok()) {
			std::cout << "Assertion failed" << std::endl;
			std::cout << s.ToString() << std::endl;
			return 1;
		}
	}

        sync();
        parse_result(2);


        /*****************PREP BUFFERS WITH RANDOM INSERTS***************/

        srand(12321);
        for (size_t i = 0; i < num_queries; i++) {
            std::string key = create_key(rand() % SEQ_WRITES, 'b');
            rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
            if (!s.ok()) std::cout << s.ToString() << std::endl;
            assert(s.ok());
        }

        sync();

	/*****************TEST RANDOM INSERTS****************************/
        
        srand(32123);
        system("blktrace -a write -d /dev/sdb4 -o rocksdb.rand.tracefile &");
        
	for(size_t i = 0; i < num_queries; i++) {
		std::string key = create_key(rand() % SEQ_WRITES, 'b');
		rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
		if (!s.ok()) std::cout << s.ToString() << std::endl;
		assert(s.ok());
	}

        sync(); 
        parse_result(3);


	/****************************CLEANUP*****************************/
	delete db;
	return 0;

}

int main(int argc, char** argv) {
	int leveldb = 0;
	int rocksdb = 0;
	int table_size = 0;
	int begin_line = 0;
	int c;

	while ((c = getopt (argc, argv, "lrbs:k:v:")) != -1 ) {
		switch (c) {
		    case 'l':
			leveldb = 1;
			break;
		    case 'r':
			rocksdb = 1;
			break;
		    case 's':
			table_size = std::stoi(optarg);
			break;
		    case 'b':
			begin_line = 1;
			break;
		    case 'k':
			FLAGS_key_size = std::stoi(optarg);
			break;
		    case 'v':
			FLAGS_value_size = std::stoi(optarg);
			break;
		    default:
			std::cout << c << optarg << std::endl;
			return 1;
		}
	}

	if (table_size == 0) {
		std::cout << "Need node size input. Use -s\n";;
		return 1;
	} else if ((leveldb && rocksdb) || (!leveldb && !rocksdb)) {
		std::cout << "Specify one of RocksDB (-r) and LevelDB (-l)\n";
		return 1;
	}

	SEQ_WRITES = 16 * 1024 * 1024 / (FLAGS_key_size + FLAGS_value_size) * 1024;
	num_queries = SEQ_WRITES;

	if (begin_line) {
		print_line_header(table_size);
	}

	if (leveldb) 
		return run_leveldb(table_size);
	if (rocksdb)
		return run_rocksdb(table_size);
	return 1;
}
