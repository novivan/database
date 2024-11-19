#include <vector>
#include <string>


//ячейка таблицы
template <class K>
class cell{
    K content;
    cell();
    ~cell();
    cell(K a): content(a){}
public:
    K get_content() {
        return this->content;
    }
    void set_content(K a) {
        content = a;
    }
};
//столбец
template <class K>
class column{
    std::string name;
    std::vector <K> cells;
    column();
    column(std::vector<K> a) {
        cells.clear();
        for (auto obj : a) cells.push_back(obj);
    }
public:
    std::vector <K> get_cells() {
        return cells;
    }
    cell<K> get_cell(int i) {
        return cells[i];
    }
    K get_cell_content(int i) {
        return cells[i].get_content();
    }

    void add_cell(K data) {
        cells.push_back(cell<K>(data));
    }
    void delete_cell(K data) {
        cells.erase(find(cells.begin(), cells.end(), data));
    }
    void delete_cell(int i) {
        if (i < cells.size()) cells.erase(cells.begin() + i);
    }

    int find_cell(K data) {
        int ret = find(cells.begin(), cells.end(), data) - cells.begin();
        if (ret < cells.size() && ret >= 0) return ret;
        return -1;
    }
    
};

//таблица
class Table{
    std::string name;
    //union нужен, тк типы столбцов могут быть разные
    typedef union {
        long double ld;
        double d;
        long long ll;
        float f;
        int i;
        char *s;
        // etc
    } un;
    std::vector <column<un> > columns;
    std::vector <int> types;
    Table() = default;
    //~Table() = default;
    Table(std::string nm): name(nm){}
    Table(std::vector <int> tps, std::string nm);
    Table(std::vector <column <un> > columns, std::string nm);
public:
    void insert_column(column<un>);
    void delete_column(int index);
    void delete_column(un param, int type);

    Table select_columns(std::vector <int> indexes);
    Table select_rows(std::vector <int> indexes);
};

//сама база
class DB {
    std::string name;
    std::vector <Table> Tables;
    DB() = default;
    ~DB() = default;
    DB(std::string nm): name(nm){}
    DB(std::vector <Table> tables, std::string name);
    DB(Table table, std::string name);
public:
    void add_table(Table table);
    void delete_table(std::string name);
    DB select_tables(std::vector <std::string> names);
};

