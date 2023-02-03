#include <iostream>
#include <filesystem>
#include <lmdb.h>

using namespace std;
namespace fs = std::filesystem;

template <typename T>
inline MDB_val to_val(T &&value) noexcept

{
    return MDB_val{sizeof(T), &value};
}

int main()
{
    string db_path = "home/toretto/project/faucetdb/database"; 
    fs::create_directories(db_path);

    // create the environment
    MDB_env *obj = nullptr;
    int rc = mdb_env_create(addressof(obj));
    if (rc != 0)
    {
        cerr << "Failed to create env" << mdb_strerror(rc) << "\n";
    }
    // set environment map size
    mdb_env_set_mapsize(obj, 10485760);
    if (rc != 0)
    {
        cerr << "Failed to set mapsize" << mdb_strerror(rc) << "\n";
    }
    // open the environment
    rc = mdb_env_open(obj, db_path.c_str(), 0, 0664);
    if (rc != 0)
    {
        cerr << "Failed to env open" << mdb_strerror(rc) << endl;
    }
    // create a new transaction
    MDB_txn *txn = nullptr;
    rc = mdb_txn_begin(obj, NULL, 0, &txn);
    if (rc != 0)
    {
        std::cerr << "failed to begin the transaction" << mdb_strerror(rc) << "\n";
        return 1;
    }
    // create a database
    MDB_dbi dbi;
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc != 0)
    {
        std::cerr << "failed to open the dbi" << mdb_strerror(rc) << "\n";
        return 1;
    }
    // read the database
    {
        // Create a cursor
        MDB_cursor *cursor;
        rc = mdb_cursor_open(txn, dbi, &cursor);
        if (rc != 0)
        {
            std::cout << "Error creating cursor: " << mdb_strerror(rc) << std::endl;
            return 1;
        }

        // Iterate through all key-value pairs
        MDB_val key, data;
        // rc = mdb_get(txn, dbi, &key, &data);
        while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0)
        {
            // Print the key and value
            std::string keyInt = static_cast<char *>(key.mv_data);
            int value = *static_cast<int *>(data.mv_data);
            std::cout << "Key: " << keyInt << ", Value: " << value << std::endl;
        }

        if (rc != MDB_NOTFOUND)
        {
            std::cout << "Error iterating through key-value pairs: " << mdb_strerror(rc) << std::endl;
        }

        // Close the cursor
        mdb_cursor_close(cursor);
    }

    // Add a key-value pair to the database
    MDB_val key, value;
    std::string name{}, loop = "yes";
    int age;
    while (loop == "yes")
    {
        std::cout << "enter a name : ";
        std::cin >> name;
        std::cout << "enter your age : ";
        std::cin >> age;

        key = to_val(name);
        value = to_val(age);
        rc = mdb_put(txn, dbi, &key, &value, 0);
        if (rc != 0)
        {
            std::cerr << "Error adding key-value pair: " << mdb_strerror(rc) << std::endl;
            return 1;
        }
        loop = {};
        while (!(loop == "yes" || loop == "no"))
        {
            std::cout << "do you want to add new key-pair (yes/no):";
            std::cin >> loop;
        }
    }

    // close the database operations
    mdb_txn_commit(txn);
    mdb_dbi_close(obj, dbi);

    // close the environment
    mdb_env_close(obj);
    return 0;
}