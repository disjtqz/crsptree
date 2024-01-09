#include <cstddef>
#include <cstdint>
#include <climits>
#include <intrin.h>
#include "crsptree.hpp"
#include "crsptree32_based.hpp"
#include <utility>
#include <Windows.h>

using crsptree_impl = crsptree32_based;

using rbnode_t = typename crsptree_impl::rbnode_t;
using insertion_position_t = typename crsptree_impl::insertion_position_t;

static unsigned char* program_base = nullptr;
struct testdata_t {
	const char* text;
	int numberkey;

	rbnode_t m_node;

	bool marker1;
	constexpr testdata_t(const char* txt, int key) : text(txt), numberkey(key), m_node{}, marker1(false) {}
};
static testdata_t testdataxxx[] = {
	{"hello", 4945},
	{"goodbye", 69},
	{"nice car", 492932398},
	{"the number 4 getting high", 420},
	{"sherbert", 68},

	{"clemency", 70},

	{"hello2", -8},
	{"goodbye2", 93993},
	{"nice car2", 0},
	{"the number 4 getting high2", 15},
	{"sherbert44", 680},

	{"54242clemency", 71}
};
static uint32_t testdata_root = 0;
static testdata_t testdata_null_filter{ "", 0 };

static std::pair<testdata_t*, insertion_position_t> find_with_hint(int key) {
	std::pair<testdata_t*, insertion_position_t> result{};

	result.first = crsptree_impl::find_entry_for_intrusive_tree<testdata_t, crsptree_offsetof_m(testdata_t, m_node)>(program_base, &testdata_root, key, [](testdata_t* currtestdata, int r) {
		return currtestdata->numberkey - r;
		}, &result.second);
	return result;
}
void clear_test_marker() {
	for (auto&& datum : testdataxxx) {
		datum.marker1 = false;
	}
}

void mark_all_forward() {

	clear_test_marker();

	rbnode_t* currnode = crsptree_impl::head_node(program_base, &testdata_root);

	while (currnode) {

		testdata_t* tdata = crsptree_containing_record_m(currnode, testdata_t, m_node);
		tdata->marker1 = true;

		currnode = crsptree_impl::next_node(program_base, currnode);
	}
}


void mark_all_reverse() {
	clear_test_marker();

	rbnode_t* currnode = crsptree_impl::tail_node(program_base, &testdata_root);

	while (currnode) {
		testdata_t* tdata = crsptree_containing_record_m(currnode, testdata_t, m_node);
		tdata->marker1 = true;

		currnode = crsptree_impl::prev_node(program_base, currnode);
	}
}

void verify_all_marked_except(testdata_t* excl = &testdata_null_filter) {
	for (auto& tdata : testdataxxx) {

		if (&tdata == excl) {
			continue;
		}

		if (!tdata.marker1) {
			__debugbreak();
		}
	}

	if (excl) {
		if (excl->marker1) {
			__debugbreak();
		}
	}
}

void verify_node_findable(testdata_t* node) {
	if (find_with_hint(node->numberkey).first != node) {
		__debugbreak();
	}
}

void verify_node_not_findable(testdata_t* node) {
	if (find_with_hint(node->numberkey).first != nullptr) {
		__debugbreak();
	}
}
void verify_all_other_nodes_findable(testdata_t* exclude) {
	//make sure every other node can still be found
	for (auto& originaldata : testdataxxx) {

		if (&originaldata == exclude) {
			continue;
		}
		if (find_with_hint(originaldata.numberkey).first != &originaldata) {
			__debugbreak();
		}

	}

}
void reinsert_node(testdata_t* node) {
	node->m_node.initialize(program_base);
	auto [unusedptr, tmphint] = find_with_hint(node->numberkey);

	tmphint.insert(program_base, &node->m_node, &testdata_root);

}
void test_tree_erasure(testdata_t* node_to_test_with) {

	mark_all_forward();
	verify_all_marked_except();

	mark_all_reverse();
	verify_all_marked_except();

	verify_node_findable(node_to_test_with);

    crsptree_impl::erase_node(program_base, &node_to_test_with->m_node, &testdata_root);
	verify_node_not_findable(node_to_test_with);

	mark_all_forward();
	verify_all_marked_except(node_to_test_with);

	mark_all_reverse();
	verify_all_marked_except(node_to_test_with);


	verify_all_other_nodes_findable(node_to_test_with);
	reinsert_node(node_to_test_with);

	mark_all_forward();
	verify_all_marked_except();

	mark_all_reverse();
	verify_all_marked_except();

	verify_all_other_nodes_findable(node_to_test_with);
	verify_node_findable(node_to_test_with);

}

int main() {

    program_base = (unsigned char*)GetModuleHandleA(nullptr);
	for (auto& entry : testdataxxx) {
		reinsert_node(&entry);
	}

	for (auto&& entry : testdataxxx) {

		insertion_position_t tmphint{  };

		auto [gotentry, unusedhint] = find_with_hint(entry.numberkey);

		if (!gotentry) {
			__debugbreak();
		}
		if (gotentry != &entry) {
			__debugbreak();
		}
	}

	for (auto&& entry : testdataxxx) {
		test_tree_erasure(&entry);

	}

	__debugbreak();//if hit, we're good.

	return 0;


}