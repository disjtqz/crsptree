
/*
	input macros - 
	CRSPTREE_NAMESPACE - the namespace this instantiation will live in

	CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE( ... )	- creates a param list for a function prototype with the memoryspace as the first arg
	CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(...)		- expands to the param list for a function, with memory_space passed as arg1

	CRSPTREE_MEMORYSPACE_PARAM						- for functions that normally would take 0 arguments, expands to just memory_space_t* memory_space
	CRSPTREE_PASS_MEMORYSPACE_PARAM					- for invoking the functions marked with CRSPTREE_MEMORYSPACE_PARAM

	CRSPTREE_PACKED_POINTER_TYPE(pointee)			
	CRSPTREE_PACKED_UINTPTR_TYPE()


	CRSPTREE_TRANSLATE_POINTER(type, ...)			- converts a packed pointer for a memory space into an absolute one

	CRSPTREE_UNTRANSLATE_POINTER(...)				- converts an unpacked pointer to a packed one

	CRSPTREE_NULL_POINTER							- the value of a null pointer for this instantiation 

	CRSPTREE_ENABLE_ITERATORS						- enables/disables forward_iterate function, which does not support memory views atm


*/

namespace CRSPTREE_NAMESPACE {
	//in the future, we may want to support 32-bit pointers within 64 bit processes
	using crsptree_uptr_t = CRSPTREE_PACKED_UINTPTR_TYPE();


	static constexpr crsptree_uptr_t CRSPTREE_COLOR_MASK = 1;
	static constexpr crsptree_uptr_t CRSPTREE_PARENT_MASK = ~CRSPTREE_COLOR_MASK;


	struct rbnode_t;
	struct rbnode_parent_t {
		crsptree_uptr_t m_value;

		rbnode_t* node(CRSPTREE_MEMORYSPACE_PARAM) {
			return CRSPTREE_TRANSLATE_POINTER(rbnode_t, m_value & CRSPTREE_PARENT_MASK);
		}

		const rbnode_t* node(CRSPTREE_MEMORYSPACE_PARAM) const {
			return CRSPTREE_TRANSLATE_POINTER(rbnode_t, m_value & CRSPTREE_PARENT_MASK);
		}
		crsptree_uptr_t color() const {
			return m_value & CRSPTREE_COLOR_MASK;
		}
		bool black() const {
			return color() != 0;
		}
		bool red() const {
			return color() == 0;
		}
		void set_color(crsptree_uptr_t new_color) {
			m_value = (m_value & CRSPTREE_PARENT_MASK) | new_color;
		}
		void set_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* nde)) {
			m_value =(crsptree_uptr_t)(CRSPTREE_UNTRANSLATE_POINTER(nde)) | (m_value & CRSPTREE_COLOR_MASK);
		}

		operator crsptree_uptr_t() const {
			return m_value;
		}
		rbnode_parent_t& operator = (crsptree_uptr_t v) {
			m_value = v;
			return *this;
		}
	};

	struct rbnode_t
	{

		//right to left ordering of children
#if defined(CRSPTREE_PARENT_FIRST)
		rbnode_parent_t m_parent;
		CRSPTREE_PACKED_POINTER_TYPE(rbnode_t) m_right_to_left[2];
#else
		rbnode_t* m_right_to_left[2];
		rbnode_parent_t m_parent;
#endif
		bool black() const {
			return m_parent.black();
		}
		bool red() const {
			return m_parent.red();
		}

		void blacken() {
			set_color(1);
		}
		void redden() {
			set_color(0);
		}
		crsptree_uptr_t color() const {
			return m_parent.color();
		}

		void set_color(crsptree_uptr_t new_color) {
			m_parent.set_color(new_color);
		}
		void set_parent_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* node)) {
			m_parent.set_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(node));
		}
		rbnode_t* parent(CRSPTREE_MEMORYSPACE_PARAM) {
			return m_parent.node(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		}
		rbnode_t* right(CRSPTREE_MEMORYSPACE_PARAM) {
			return CRSPTREE_TRANSLATE_POINTER(rbnode_t, m_right_to_left[0]);
		}
		rbnode_t* left(CRSPTREE_MEMORYSPACE_PARAM) {
			return CRSPTREE_TRANSLATE_POINTER(rbnode_t, m_right_to_left[1]);
		}

		void set_right(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* new_right)) {
			m_right_to_left[0] = CRSPTREE_UNTRANSLATE_POINTER(new_right);
		}
		void set_left(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* new_left)) {
			m_right_to_left[1] = CRSPTREE_UNTRANSLATE_POINTER(new_left);
		}

		const rbnode_t* parent(CRSPTREE_MEMORYSPACE_PARAM) const {
			return m_parent.node(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		}
		const rbnode_t* right(CRSPTREE_MEMORYSPACE_PARAM) const {
			return CRSPTREE_TRANSLATE_POINTER(const rbnode_t, m_right_to_left[0]);
		}
		const rbnode_t* left(CRSPTREE_MEMORYSPACE_PARAM) const {
			return CRSPTREE_TRANSLATE_POINTER(const rbnode_t, m_right_to_left[1]);
		}

		static inline void link(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* node, rbnode_t* parent,
			CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* rb_link))
		{
			node->m_parent.m_value = (crsptree_uptr_t)(CRSPTREE_UNTRANSLATE_POINTER(parent));
			node->m_right_to_left[0] = CRSPTREE_NULL_POINTER;
			node->m_right_to_left[1] = CRSPTREE_NULL_POINTER;

			*rb_link = CRSPTREE_UNTRANSLATE_POINTER(node);
		}

		void initialize(CRSPTREE_MEMORYSPACE_PARAM) {
			m_parent.m_value = 0U;
			m_right_to_left[0] = CRSPTREE_NULL_POINTER;
			m_right_to_left[1] = CRSPTREE_NULL_POINTER;
			m_parent.set_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(this));
		}

		static constexpr uint32_t invert_lr_offset(uint32_t offset) {
#if defined(CRSPTREE_PARENT_FIRST)
			return offset ^ sizeof(rbnode_t);
#else
			static_assert(false, "need to handle for packed pointer");
			return offset ^ 0x8;
#endif
		}

		CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)& subnode_from_offset(uint32_t offset) {
			return *reinterpret_cast<CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)*>(reinterpret_cast<char*>(this) + offset);
		}
		CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)& index_child(uint32_t index) {
			return m_right_to_left[index];
		}
	};
	static constexpr uint32_t node_offsetof_right = crsptree_offsetof_m(rbnode_t, m_right_to_left[0]);
	static constexpr uint32_t node_offsetof_left = crsptree_offsetof_m(rbnode_t, m_right_to_left[1]);

	static uint32_t offset_for_left_if_neq(rbnode_t* x, rbnode_t* y) {
		uint32_t offset = node_offsetof_right;

		offset += static_cast<uint32_t>(x != y) * sizeof(CRSPTREE_PACKED_UINTPTR_TYPE());
		return offset;
	}
	static uint32_t offset_for_left_if_eq(rbnode_t* x, rbnode_t* y) {
		uint32_t offset = node_offsetof_right;

		offset += static_cast<uint32_t>(x == y) * sizeof(CRSPTREE_PACKED_UINTPTR_TYPE());
		return offset;
	}
	namespace _detail {
		constexpr uint32_t offsetof_left = crsptree_offsetof_m(rbnode_t, m_right_to_left[1]);
		constexpr uint32_t offsetof_right = crsptree_offsetof_m(rbnode_t, m_right_to_left[0]);

		static_assert(rbnode_t::invert_lr_offset(offsetof_left) == offsetof_right);
		static_assert(rbnode_t::invert_lr_offset(offsetof_right) == offsetof_left);
	}
	CRSPTREE_PUREISH
	CRSPTREE_NOINLINE
	static rbnode_t* heads_or_tails(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* tree_root, uint32_t offset_for_first_or_last))
	{
		rbnode_t* parent = CRSPTREE_TRANSLATE_POINTER(rbnode_t, *tree_root);
		if (!parent)
			return nullptr;
		rbnode_t* result;
		do
		{
			result = parent;
			rbnode_t* parent_subnode = CRSPTREE_TRANSLATE_POINTER(rbnode_t, parent->subnode_from_offset(offset_for_first_or_last));
			if (!parent_subnode)
				break;
			result = parent_subnode;
			parent = CRSPTREE_TRANSLATE_POINTER(rbnode_t, parent_subnode->subnode_from_offset(offset_for_first_or_last));
		} while (parent);
		return result;
	}

	//get first node in tree
	static rbnode_t* tail_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* tree_root))
	{
		return heads_or_tails(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(tree_root, node_offsetof_right));
	}
	//get last node in tree
	static
		rbnode_t* head_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* tree_root))
	{
		return heads_or_tails(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(tree_root, node_offsetof_left));
	}


	CRSPTREE_PUREISH
	CRSPTREE_NOINLINE
	static rbnode_t* next_or_prev(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* iter_start, uint32_t dir))
	{
		rbnode_t* iter = iter_start->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		if (iter == iter_start) //todo: how likely is this?
			return nullptr;
		rbnode_t* iter_initial = CRSPTREE_TRANSLATE_POINTER(rbnode_t, iter_start->subnode_from_offset(dir));
		if (iter_initial)
		{
			uint32_t inverted_dir = rbnode_t::invert_lr_offset(dir);
			rbnode_t* iter_unchecked = iter_initial;
			//unrolled this by one iter
			do
			{
				iter = iter_unchecked;
				rbnode_t* iter_unroll1 = CRSPTREE_TRANSLATE_POINTER(rbnode_t, iter_unchecked->subnode_from_offset(inverted_dir));
				if (!iter_unroll1)
					break;
				iter = iter_unroll1;
				iter_unchecked = CRSPTREE_TRANSLATE_POINTER(rbnode_t, iter_unroll1->subnode_from_offset(inverted_dir));
			} while (iter_unchecked);
			return iter;
		}
		if (!iter)
			return nullptr;
		//ascend through parents until we find the first one on the side we want
		while (iter_start == CRSPTREE_TRANSLATE_POINTER(rbnode_t, iter->subnode_from_offset(dir)))
		{
			iter_start = iter;
			iter = iter->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
			if (!iter)
				return nullptr;
		}
		return iter;
	}

	CRSPTREE_FORCEINLINE
		static rbnode_t* next_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* current_node)) {
		return next_or_prev(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(current_node, node_offsetof_right));
	}

	CRSPTREE_FORCEINLINE
		static rbnode_t* prev_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* current_node)) {
		return next_or_prev(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(current_node, node_offsetof_left));
	}

	CRSPTREE_NOINLINE
		static
		void rotate_by_offset(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* node, uint32_t offset, CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* root))
	{
		uint32_t inv_offset = rbnode_t::invert_lr_offset(offset);
		rbnode_t* child_for_offset = CRSPTREE_TRANSLATE_POINTER(rbnode_t, node->subnode_from_offset(offset));
		rbnode_t* input_nodes_parent = node->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		rbnode_t* child_for_inverse_offset = CRSPTREE_TRANSLATE_POINTER(rbnode_t, child_for_offset->subnode_from_offset(inv_offset));
		node->subnode_from_offset(offset) = CRSPTREE_UNTRANSLATE_POINTER(child_for_inverse_offset);


		if (child_for_inverse_offset) {
			CRSPTREE_TRANSLATE_POINTER(rbnode_t, child_for_offset->subnode_from_offset(inv_offset))->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(node));
		}
		child_for_offset->subnode_from_offset(inv_offset) = CRSPTREE_UNTRANSLATE_POINTER(node);

		child_for_offset->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(input_nodes_parent));
		if (input_nodes_parent) {
			//codegen for this index_child is fine
			root = &input_nodes_parent->index_child(input_nodes_parent->right(CRSPTREE_PASS_MEMORYSPACE_PARAM) != node);
		}
		*root = CRSPTREE_UNTRANSLATE_POINTER(child_for_offset);
		node->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(child_for_offset));
	}
	CRSPTREE_NOINLINE
		static void insert_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* node_to_insert, CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* tree_root))
	{
		rbnode_t* current_node = node_to_insert;
		rbnode_t* inserted_nodes_parent = node_to_insert->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		if (inserted_nodes_parent)
		{
			while (true)
			{
				rbnode_parent_t parent = inserted_nodes_parent->m_parent;
				if (inserted_nodes_parent->black())
					break;
				rbnode_t* current_parent_node = parent.node(CRSPTREE_PASS_MEMORYSPACE_PARAM);

				uint32_t current_parent_node_subnode_offset = offset_for_left_if_neq(current_parent_node->left(CRSPTREE_PASS_MEMORYSPACE_PARAM), inserted_nodes_parent);
				uint32_t inv_current_parent_node_subnode_offset = rbnode_t::invert_lr_offset(current_parent_node_subnode_offset);
				rbnode_t* current_parent_node_subnode = CRSPTREE_TRANSLATE_POINTER(rbnode_t, current_parent_node->subnode_from_offset(current_parent_node_subnode_offset));

				if (!current_parent_node_subnode || current_parent_node_subnode->black())
				{
					if (CRSPTREE_TRANSLATE_POINTER(rbnode_t, inserted_nodes_parent->subnode_from_offset(current_parent_node_subnode_offset)) == current_node)
					{
						rbnode_t* inserted_nodes_parent_backup = inserted_nodes_parent;
						rotate_by_offset(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(inserted_nodes_parent, current_parent_node_subnode_offset, tree_root));
						parent = current_node->m_parent;
						inserted_nodes_parent = current_node;
						current_node = inserted_nodes_parent_backup;
					}
					inserted_nodes_parent->m_parent = parent;
					inserted_nodes_parent->blacken();
					current_parent_node->redden();

					rotate_by_offset(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(current_parent_node, inv_current_parent_node_subnode_offset, tree_root));
					inserted_nodes_parent = current_node->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
					if (!inserted_nodes_parent)
						break;
				}
				else
				{
					current_node = parent.node(CRSPTREE_PASS_MEMORYSPACE_PARAM);
					current_parent_node_subnode->blacken();
					inserted_nodes_parent->blacken();
					current_parent_node->redden();

					inserted_nodes_parent = current_parent_node->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
					if (!inserted_nodes_parent)
						break;
				}
			}
		}
		CRSPTREE_TRANSLATE_POINTER(rbnode_t, *tree_root)->blacken();
	}

	CRSPTREE_FORCEINLINE
		static rbnode_t* crawl_left(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* input_right)) {
		rbnode_t* leftmost_node;
		do
		{
			leftmost_node = input_right;
			input_right = input_right->left(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		} while (input_right);
		return leftmost_node;
	}
	CRSPTREE_NOINLINE
		static void erase_node(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* node_to_erase, CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* tree_root))
	{


		rbnode_parent_t parent;
		rbnode_t* erased_node_parent;
		rbnode_t* input_right = node_to_erase->right(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		rbnode_t* node_child = node_to_erase->left(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		if (!node_child)
		{
			parent = node_to_erase->m_parent;
			erased_node_parent = node_to_erase->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
			if (!input_right)
			{
				node_child = nullptr;
				if (erased_node_parent)
				{
				handle_erasure_of_normal_node:
					erased_node_parent->index_child(erased_node_parent->left(CRSPTREE_PASS_MEMORYSPACE_PARAM) == node_to_erase) = CRSPTREE_UNTRANSLATE_POINTER(node_child);
					if (parent.black())
						goto recolor;
					return;
				}
			handle_erasure_of_root:
				*tree_root = CRSPTREE_UNTRANSLATE_POINTER(node_child);
				erased_node_parent = nullptr;
				if (parent.black())
					goto recolor;
				return;
			}
			node_child = node_to_erase->right(CRSPTREE_PASS_MEMORYSPACE_PARAM);
		handle_left_childs_new_parent:
			node_child->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(erased_node_parent));
			if (erased_node_parent)
				goto handle_erasure_of_normal_node;
			goto handle_erasure_of_root;
		}
		if (!input_right)
		{
			//skip over adjustment of right child
			parent = node_to_erase->m_parent;
			erased_node_parent = node_to_erase->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
			goto handle_left_childs_new_parent;
		}

		{
			auto leftmost_node = crawl_left(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(input_right));

			CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* rightchild_writepos;
			auto erased_parent = node_to_erase->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);

			if (erased_parent) {//this branch is more likely than the root one

				rightchild_writepos = &erased_parent->subnode_from_offset(offset_for_left_if_eq(erased_parent->left(CRSPTREE_PASS_MEMORYSPACE_PARAM), node_to_erase));
			}
			else {
				rightchild_writepos = tree_root;
			}
			*rightchild_writepos = CRSPTREE_UNTRANSLATE_POINTER(leftmost_node);
			{
				rbnode_parent_t leftmost_parent = leftmost_node->m_parent;
				node_child = leftmost_node->right(CRSPTREE_PASS_MEMORYSPACE_PARAM);
				erased_node_parent = leftmost_node->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
				if (erased_node_parent == node_to_erase)
				{
					erased_node_parent = leftmost_node;
				}
				else
				{
					if (node_child)
						node_child->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(erased_node_parent));
					erased_node_parent->set_left(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(node_child));
					leftmost_node->set_right(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(node_to_erase->right(CRSPTREE_PASS_MEMORYSPACE_PARAM)));

					node_to_erase->right(CRSPTREE_PASS_MEMORYSPACE_PARAM)->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(leftmost_node));
				}
				leftmost_node->m_parent = node_to_erase->m_parent;
				leftmost_node->set_left(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(node_to_erase->left(CRSPTREE_PASS_MEMORYSPACE_PARAM)));
				node_to_erase->left(CRSPTREE_PASS_MEMORYSPACE_PARAM)->set_parent_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(leftmost_node));
				if (!leftmost_parent.black()) {
					return;
				}
			}
		}
		recolor :
		{


			rbnode_parent_t resulting_parent;
			uint32_t offset_for_dual_rotate_label;
			rbnode_t* current_node = node_child;

			while (true)
			{

				if (current_node)
				{
					resulting_parent = current_node->m_parent;
					if (resulting_parent.red())
						break;
				}
				if (CRSPTREE_UNTRANSLATE_POINTER(current_node) != *tree_root) {

					rbnode_t* parent_left = erased_node_parent->left(CRSPTREE_PASS_MEMORYSPACE_PARAM);
					uint32_t parent_subnode_offset = offset_for_left_if_neq(parent_left, current_node);
					rbnode_t* parent_subnode = CRSPTREE_TRANSLATE_POINTER(rbnode_t, erased_node_parent->subnode_from_offset(parent_subnode_offset));
					if (parent_subnode->red())
					{

						parent_subnode->blacken();
						erased_node_parent->redden();
						rotate_by_offset(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(erased_node_parent, parent_subnode_offset, tree_root));
						parent_subnode = CRSPTREE_TRANSLATE_POINTER(rbnode_t, erased_node_parent->subnode_from_offset(parent_subnode_offset));
					}
					rbnode_t* parent_subnode_left = parent_subnode->left(CRSPTREE_PASS_MEMORYSPACE_PARAM);
					if (parent_subnode_left && parent_subnode_left->red())
					{
						if (parent_left == current_node)
						{
							offset_for_dual_rotate_label = node_offsetof_left;
							if (!parent_subnode->right(CRSPTREE_PASS_MEMORYSPACE_PARAM) || parent_subnode->right(CRSPTREE_PASS_MEMORYSPACE_PARAM)->black())
							{
							dual_rotate:
								{
									auto eqnodeoffs = offset_for_left_if_eq(parent_left, current_node);
									CRSPTREE_TRANSLATE_POINTER(rbnode_t, parent_subnode->subnode_from_offset(offset_for_dual_rotate_label))->blacken();
									parent_subnode->redden();
									rotate_by_offset(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(parent_subnode, eqnodeoffs, tree_root));
									parent_subnode = CRSPTREE_TRANSLATE_POINTER(rbnode_t, erased_node_parent->subnode_from_offset(rbnode_t::invert_lr_offset(eqnodeoffs)));
								}
							}
						}
					single_rotate:
						{
							uint32_t offset2 = offset_for_left_if_neq(parent_left, current_node);
							parent_subnode->set_color(erased_node_parent->color());
							erased_node_parent->blacken();

							CRSPTREE_TRANSLATE_POINTER(rbnode_t, parent_subnode->subnode_from_offset(offset2))->blacken();
							rotate_by_offset(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(erased_node_parent, offset2, tree_root));
						}
						current_node = CRSPTREE_TRANSLATE_POINTER (rbnode_t, *tree_root);
						if (!current_node)
							return;
						resulting_parent = current_node->m_parent;
						break;
					}
					if (parent_subnode->right(CRSPTREE_PASS_MEMORYSPACE_PARAM) && parent_subnode->right(CRSPTREE_PASS_MEMORYSPACE_PARAM)->red())
					{
						if (parent_left != current_node && (!parent_subnode_left || parent_subnode_left->black()))
						{
							offset_for_dual_rotate_label = node_offsetof_right;
							goto dual_rotate;
						}
						goto single_rotate;
					}
					parent_subnode->redden();
					current_node = erased_node_parent;
					erased_node_parent = erased_node_parent->parent(CRSPTREE_PASS_MEMORYSPACE_PARAM);
				}
				else {
					if (!current_node)
						return;

					resulting_parent = current_node->m_parent;
					break;
				}
			}
			current_node->m_parent = resulting_parent;
			current_node->blacken();
		}
	}



	struct insertion_position_t {
		CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* m_current_node_ptr;
		rbnode_t* m_current_parent;

		void insert(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(rbnode_t* new_node, CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* into_tree)) {
			rbnode_t::link(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(new_node, m_current_parent, m_current_node_ptr));
			insert_node(CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE(new_node, into_tree));
		}
	};

	template<typename TContainer, uint32_t containing_record_offset, typename LambdaRightArgType, typename LambdaCompareType>
	CRSPTREE_FORCEINLINE
		static TContainer* find_entry_for_intrusive_tree(CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE(CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* root, LambdaRightArgType lambda_right_arg, LambdaCompareType&& compare_function, insertion_position_t* hint)) {
		CRSPTREE_PACKED_POINTER_TYPE(rbnode_t)* iterator_position = root;

		rbnode_t* parent = nullptr;

		while (CRSPTREE_TRANSLATE_POINTER(rbnode_t, *iterator_position) ) {
			rbnode_t* current_node = CRSPTREE_TRANSLATE_POINTER(rbnode_t, *iterator_position);



			TContainer* container_of_node = reinterpret_cast<TContainer*>(
				reinterpret_cast<char*>(current_node) - containing_record_offset
				);

			ptrdiff_t sign_of_comparison = compare_function(container_of_node, lambda_right_arg);
			parent = CRSPTREE_TRANSLATE_POINTER(rbnode_t, *iterator_position);
			if (sign_of_comparison) {
				iterator_position = &current_node->index_child(sign_of_comparison > 0);
			}
			else {
				return container_of_node;
			}

		}

		if (hint) {
			hint->m_current_node_ptr = iterator_position;
			hint->m_current_parent = parent;
		}
		return nullptr;
	}
#if CRSPTREE_ENABLE_ITERATORS==1
	template<typename TContainer, unsigned delta_from_node_to_object>
	struct next_node_iterator_t {
		rbnode_t* m_node;
		inline TContainer* operator *() {
			return reinterpret_cast<TContainer*>(reinterpret_cast<char*>(m_node) - delta_from_node_to_object);
		}
		inline next_node_iterator_t<TContainer, delta_from_node_to_object>& operator ++() {
			m_node = next_node(m_node);
			return *this;
		}
		constexpr bool operator !=(std::nullptr_t p) const {
			return m_node != p;
		}
	};

	template<typename TContainer, unsigned delta_from_node_to_object>
	struct rb_nodes_iter_t {

		rbnode_t** m_current;

		rb_nodes_iter_t(rbnode_t*& root) : m_current(&root) {}


		std::nullptr_t end() {
			return nullptr;
		}

		const std::nullptr_t end() const {
			return nullptr;
		}

		next_node_iterator_t<TContainer, delta_from_node_to_object> begin() {
			return next_node_iterator_t<TContainer, delta_from_node_to_object>{head_node(m_current) };
		}

	};
	template<typename TContainer, unsigned delta_from_node_to_object>
	static inline rb_nodes_iter_t<TContainer, delta_from_node_to_object> iterate_forward(rbnode_t*& root) {
		return rb_nodes_iter_t<TContainer, delta_from_node_to_object>{root};
	}
#endif
}

//clear all macros

#undef     CRSPTREE_NAMESPACE


#undef     CRSPTREE_DEFINE_PARAMS_WITH_MEMORYSPACE
#undef     CRSPTREE_PASS_PARAMS_WITH_MEMORYSPACE

#undef     CRSPTREE_MEMORYSPACE_PARAM
#undef     CRSPTREE_PASS_MEMORYSPACE_PARAM
#undef     CRSPTREE_PACKED_POINTER_TYPE
#undef     CRSPTREE_PACKED_UINTPTR_TYPE

#undef     CRSPTREE_NULL_POINTER          

#undef     CRSPTREE_TRANSLATE_POINTER

#undef     CRSPTREE_UNTRANSLATE_POINTER

#undef     CRSPTREE_ENABLE_ITERATORS