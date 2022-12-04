#pragma once
/*
	This header requires that you include cstddef cstdint and climits
*/



//compiler-specific features for better codegen


#if defined(_MSC_VER)
#define		CRSPTREE_NOINLINE		__declspec(noinline)
#define		CRSPTREE_FORCEINLINE	__forceinline
#define		CRSPTREE_RESTRICT		__restrict
#define		CRSPTREE_PUREISH		__declspec(noalias)
#elif defined(__GNUG__)
#define		CRSPTREE_NOINLINE		__attribute__((noinline))
#define		CRSPTREE_FORCEINLINE	__attribute__((always_inline))
#define		CRSPTREE_RESTRICT		__restrict
#define		CRSPTREE_PUREISH		__attribute__((pure))
#else
#define		CRSPTREE_NOINLINE		
#define		CRSPTREE_FORCEINLINE	inline
#define		CRSPTREE_RESTRICT		
#define		CRSPTREE_PUREISH
#endif
#define		crsptree_offsetof_m		__builtin_offsetof
#define		crsptree_containing_record_m(p, container, memb)	reinterpret_cast<container *>( reinterpret_cast<char *>(p) - crsptree_offsetof_m(container, memb))
//end compiler-specific features

#define		CRSPTREE_PARENT_FIRST		
#define		CRSPTREE_CACHELINE_SIZE		64

#if defined(__has_builtin) && __has_builtin (__builtin_prefetch)
static void prefetchL0(const void* loc) {
	__builtin_prefetch((const char*)loc);
}
#else
static void prefetchL0(const void* loc) {
	_mm_prefetch((const char*)loc, _MM_HINT_T0);
}
#endif
namespace crsptree {
	//in the future, we may want to support 32-bit pointers within 64 bit processes
	using crsptree_uptr_t = uintptr_t;


	static constexpr crsptree_uptr_t CRSPTREE_COLOR_MASK = 1ULL;
	static constexpr crsptree_uptr_t CRSPTREE_PARENT_MASK = ~CRSPTREE_COLOR_MASK;


	struct rbnode_t;
	struct rbnode_parent_t {
		crsptree_uptr_t m_value;

		rbnode_t* node() {
			return reinterpret_cast<rbnode_t*>(m_value & CRSPTREE_PARENT_MASK);
		}

		const rbnode_t* node() const {
			return reinterpret_cast<rbnode_t*>(m_value & CRSPTREE_PARENT_MASK);
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
		void set_node(rbnode_t* nde) {
			m_value = reinterpret_cast<crsptree_uptr_t>(nde) | (m_value & CRSPTREE_COLOR_MASK);
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
		rbnode_t* m_right_to_left[2];
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
		uintptr_t color() const {
			return m_parent.color();
		}

		void set_color(crsptree_uptr_t new_color) {
			m_parent.set_color(new_color);
		}
		void set_parent_node(rbnode_t* node) {
			m_parent.set_node(node);
		}
		rbnode_t* parent() {
			return m_parent.node();
		}
		rbnode_t* right() {
			return m_right_to_left[0];
		}
		rbnode_t* left() {
			return m_right_to_left[1];
		}

		void set_right(rbnode_t* new_right) {
			m_right_to_left[0] = new_right;
		}
		void set_left(rbnode_t* new_left) {
			m_right_to_left[1] = new_left;
		}

		const rbnode_t* parent() const {
			return m_parent.node();
		}
		const rbnode_t* right() const {
			return m_right_to_left[0];
		}
		const rbnode_t* left() const {
			return m_right_to_left[1];
		}

		static inline void link(rbnode_t* node, rbnode_t* parent,
			rbnode_t** rb_link)
		{
			node->m_parent.m_value = reinterpret_cast<crsptree_uptr_t>(parent);
			node->m_right_to_left[0] = nullptr;
			node->m_right_to_left[1] = nullptr;

			*rb_link = node;
		}

		void initialize() {
			m_parent.m_value = 0U;
			m_right_to_left[0] = nullptr;
			m_right_to_left[1] = nullptr;
			m_parent.set_node(this);
		}

		static constexpr uint32_t invert_lr_offset(uint32_t offset) {
#if defined(CRSPTREE_PARENT_FIRST)
			return offset ^ 0x18;
#else
			return offset ^ 0x8;
#endif
		}

		rbnode_t*& subnode_from_offset(uint32_t offset) {
			return *reinterpret_cast<rbnode_t**>(reinterpret_cast<char*>(this) + offset);
		}
		rbnode_t*& index_child(uint32_t index) {
			return m_right_to_left[index];
		}

		void unchecked_prefetch() const {
			prefetchL0(this);
		}
		bool does_cross_cacheline() const {
			uintptr_t cache_misalignment = (((uintptr_t)this) & (CRSPTREE_CACHELINE_SIZE - 1));

			return cache_misalignment + sizeof(rbnode_t) > CRSPTREE_CACHELINE_SIZE;

		}
		void checked_prefetch() const {
			prefetchL0(this);
			if (does_cross_cacheline()) {
				prefetchL0(&this[1]);
			}

		}
		void prefetch_children() const {
			if (right()) {
				right()->checked_prefetch();
			}
			if (left()) {
				left()->checked_prefetch();
			}
		}
		void prefetch_parent() {
			if (parent()) {
				parent()->checked_prefetch();
			}
		}
	};
	static constexpr uint32_t node_offsetof_right = crsptree_offsetof_m(rbnode_t, m_right_to_left[0]);
	static constexpr uint32_t node_offsetof_left = crsptree_offsetof_m(rbnode_t, m_right_to_left[1]);
	static intptr_t node_neq_mask_prelim(rbnode_t* x, rbnode_t* y) {
		//if they are both the same, both delta1 and delta will be 0, otherwise one of them will have a 1 in the signbit
		intptr_t delta1 = reinterpret_cast<intptr_t>(x) - reinterpret_cast<intptr_t>(y);
		intptr_t delta2 = reinterpret_cast<intptr_t>(y) - reinterpret_cast<intptr_t>(x);

		intptr_t delta_union = delta1 | delta2;

		return delta_union;
	}
	static intptr_t node_neq_mask(rbnode_t* x, rbnode_t* y) {
		return node_neq_mask_prelim(x, y) >> ((sizeof(intptr_t) * CHAR_BIT) - 1);
	}


	static intptr_t node_eq_mask(rbnode_t* x, rbnode_t* y) {
		return ~node_neq_mask(x, y);
	}

	static uint32_t offset_for_left_if_neq(rbnode_t* x, rbnode_t* y) {
		uint32_t offset = node_offsetof_right;

		offset += static_cast<uint32_t>(x != y) << 3;
		return offset;
	}
	static uint32_t offset_for_left_if_eq(rbnode_t* x, rbnode_t* y) {
		uint32_t offset = node_offsetof_right;

		offset += static_cast<uint32_t>(x == y) << 3;
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
	static rbnode_t* heads_or_tails(rbnode_t** tree_root, uint32_t offset_for_first_or_last)
	{
		rbnode_t* parent = *tree_root;
		if (!parent)
			return nullptr;
		rbnode_t* result;
		do
		{
			result = parent;
			rbnode_t* parent_subnode = parent->subnode_from_offset(offset_for_first_or_last);
			if (!parent_subnode)
				break;
			result = parent_subnode;
			parent = parent_subnode->subnode_from_offset(offset_for_first_or_last);
		} while (parent);
		return result;
	}

	//get first node in tree
	static rbnode_t* tail_node(rbnode_t** tree_root)
	{
		return heads_or_tails(tree_root, node_offsetof_right);
	}
	//get last node in tree
	static
		rbnode_t* head_node(rbnode_t** tree_root)
	{
		return heads_or_tails(tree_root, node_offsetof_left);
	}


	CRSPTREE_PUREISH
	CRSPTREE_NOINLINE
	static rbnode_t* next_or_prev(rbnode_t* iter_start, uint32_t dir)
	{
		rbnode_t* iter = iter_start->parent();
		if (iter == iter_start) //todo: how likely is this?
			return nullptr;
		rbnode_t* iter_initial = iter_start->subnode_from_offset(dir);
		if (iter_initial)
		{
			uint32_t inverted_dir = rbnode_t::invert_lr_offset(dir);
			rbnode_t* iter_unchecked = iter_initial;
			//unrolled this by one iter
			do
			{
				iter = iter_unchecked;
				rbnode_t* iter_unroll1 = iter_unchecked->subnode_from_offset(inverted_dir);
				if (!iter_unroll1)
					break;
				iter = iter_unroll1;
				iter_unchecked = iter_unroll1->subnode_from_offset(inverted_dir);
			} while (iter_unchecked);
			return iter;
		}
		if (!iter)
			return nullptr;
		//ascend through parents until we find the first one on the side we want
		while (iter_start == iter->subnode_from_offset(dir))
		{
			iter_start = iter;
			iter = iter->parent();
			if (!iter)
				return nullptr;
		}
		return iter;
	}

	CRSPTREE_FORCEINLINE
		static rbnode_t* next_node(rbnode_t* current_node) {
		return next_or_prev(current_node, node_offsetof_right);
	}

	CRSPTREE_FORCEINLINE
		static rbnode_t* prev_node(rbnode_t* current_node) {
		return next_or_prev(current_node, node_offsetof_left);
	}

	CRSPTREE_NOINLINE
		static
		void rotate_by_offset(rbnode_t* node, uint32_t offset, rbnode_t** root)
	{
		uint32_t inv_offset = rbnode_t::invert_lr_offset(offset);
		rbnode_t* child_for_offset = node->subnode_from_offset(offset);
		rbnode_t* input_nodes_parent = node->parent();
		rbnode_t* child_for_inverse_offset = child_for_offset->subnode_from_offset(inv_offset);
		node->subnode_from_offset(offset) = child_for_inverse_offset;


		if (child_for_inverse_offset) {
			child_for_offset->subnode_from_offset(inv_offset)->set_parent_node(node);
		}
		child_for_offset->subnode_from_offset(inv_offset) = node;

		child_for_offset->set_parent_node(input_nodes_parent);
		if (input_nodes_parent) {
			//codegen for this index_child is fine
			root = &input_nodes_parent->index_child(input_nodes_parent->right() != node);
		}
		*root = child_for_offset;
		node->set_parent_node(child_for_offset);
	}
	CRSPTREE_NOINLINE
		void insert_node(rbnode_t* node_to_insert, rbnode_t** tree_root)
	{
		rbnode_t* current_node = node_to_insert;
		rbnode_t* inserted_nodes_parent = node_to_insert->parent();
		if (inserted_nodes_parent)
		{
			while (true)
			{
				rbnode_parent_t parent = inserted_nodes_parent->m_parent;
				if (inserted_nodes_parent->black())
					break;
				rbnode_t* current_parent_node = parent.node();

				uint32_t current_parent_node_subnode_offset = offset_for_left_if_neq(current_parent_node->left(), inserted_nodes_parent);
				uint32_t inv_current_parent_node_subnode_offset = rbnode_t::invert_lr_offset(current_parent_node_subnode_offset);
				rbnode_t* current_parent_node_subnode = current_parent_node->subnode_from_offset(current_parent_node_subnode_offset);

				if (!current_parent_node_subnode || current_parent_node_subnode->black())
				{
					if (inserted_nodes_parent->subnode_from_offset(current_parent_node_subnode_offset) == current_node)
					{
						rbnode_t* inserted_nodes_parent_backup = inserted_nodes_parent;
						rotate_by_offset(inserted_nodes_parent, current_parent_node_subnode_offset, tree_root);
						parent = current_node->m_parent;
						inserted_nodes_parent = current_node;
						current_node = inserted_nodes_parent_backup;
					}
					inserted_nodes_parent->m_parent = parent;
					inserted_nodes_parent->blacken();
					current_parent_node->redden();

					rotate_by_offset(current_parent_node, inv_current_parent_node_subnode_offset, tree_root);
					inserted_nodes_parent = current_node->parent();
					if (!inserted_nodes_parent)
						break;
				}
				else
				{
					current_node = parent.node();
					current_parent_node_subnode->blacken();
					inserted_nodes_parent->blacken();
					current_parent_node->redden();

					inserted_nodes_parent = current_parent_node->parent();
					if (!inserted_nodes_parent)
						break;
				}
			}
		}
		(*tree_root)->blacken();
	}

	CRSPTREE_FORCEINLINE
		static rbnode_t* crawl_left(rbnode_t* input_right) {
		rbnode_t* leftmost_node;
		do
		{
			leftmost_node = input_right;
			input_right = input_right->left();
		} while (input_right);
		return leftmost_node;
	}
	CRSPTREE_NOINLINE
		void erase_node(rbnode_t* node_to_erase, rbnode_t** tree_root)
	{


		rbnode_parent_t parent;
		rbnode_t* erased_node_parent;
		rbnode_t* input_right = node_to_erase->right();
		rbnode_t* node_child = node_to_erase->left();
		if (!node_child)
		{
			parent = node_to_erase->m_parent;
			erased_node_parent = node_to_erase->parent();
			if (!input_right)
			{
				node_child = nullptr;
				if (erased_node_parent)
				{
				handle_erasure_of_normal_node:
					erased_node_parent->index_child(erased_node_parent->left() == node_to_erase) = node_child;
					if (parent.black())
						goto recolor;
					return;
				}
			handle_erasure_of_root:
				*tree_root = node_child;
				erased_node_parent = nullptr;
				if (parent.black())
					goto recolor;
				return;
			}
			node_child = node_to_erase->right();
		handle_left_childs_new_parent:
			node_child->set_parent_node(erased_node_parent);
			if (erased_node_parent)
				goto handle_erasure_of_normal_node;
			goto handle_erasure_of_root;
		}
		if (!input_right)
		{
			//skip over adjustment of right child
			parent = node_to_erase->m_parent;
			erased_node_parent = node_to_erase->parent();
			goto handle_left_childs_new_parent;
		}

		{
			auto leftmost_node = crawl_left(input_right);

			rbnode_t** rightchild_writepos;
			auto erased_parent = node_to_erase->parent();

			if (erased_parent) {//this branch is more likely than the root one

				rightchild_writepos = &erased_parent->subnode_from_offset(offset_for_left_if_eq(erased_parent->left(), node_to_erase));
			}
			else {
				rightchild_writepos = tree_root;
			}
			*rightchild_writepos = leftmost_node;
			{
				rbnode_parent_t leftmost_parent = leftmost_node->m_parent;
				node_child = leftmost_node->right();
				erased_node_parent = leftmost_node->parent();
				if (erased_node_parent == node_to_erase)
				{
					erased_node_parent = leftmost_node;
				}
				else
				{
					if (node_child)
						node_child->set_parent_node(erased_node_parent);
					erased_node_parent->set_left(node_child);
					leftmost_node->set_right(node_to_erase->right());

					node_to_erase->right()->set_parent_node(leftmost_node);
				}
				leftmost_node->m_parent = node_to_erase->m_parent;
				leftmost_node->set_left(node_to_erase->left());
				node_to_erase->left()->set_parent_node(leftmost_node);
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
				if (current_node != *tree_root) {

					rbnode_t* parent_left = erased_node_parent->left();
					uint32_t parent_subnode_offset = offset_for_left_if_neq(parent_left, current_node);
					rbnode_t* parent_subnode = erased_node_parent->subnode_from_offset(parent_subnode_offset);
					if (parent_subnode->red())
					{

						parent_subnode->blacken();
						erased_node_parent->redden();
						rotate_by_offset(erased_node_parent, parent_subnode_offset, tree_root);
						parent_subnode = erased_node_parent->subnode_from_offset(parent_subnode_offset);
					}
					rbnode_t* parent_subnode_left = parent_subnode->left();
					if (parent_subnode_left && parent_subnode_left->red())
					{
						if (parent_left == current_node)
						{
							offset_for_dual_rotate_label = node_offsetof_left;
							if (!parent_subnode->right() || parent_subnode->right()->black())
							{
							dual_rotate:
								{
									auto eqnodeoffs = offset_for_left_if_eq(parent_left, current_node);
									parent_subnode->subnode_from_offset(offset_for_dual_rotate_label)->blacken();
									parent_subnode->redden();
									rotate_by_offset(parent_subnode, eqnodeoffs, tree_root);
									parent_subnode = erased_node_parent->subnode_from_offset(rbnode_t::invert_lr_offset(eqnodeoffs));
								}
							}
						}
					single_rotate:
						{
							uint32_t offset2 = offset_for_left_if_neq(parent_left, current_node);
							parent_subnode->set_color(erased_node_parent->color());
							erased_node_parent->blacken();

							parent_subnode->subnode_from_offset(offset2)->blacken();
							rotate_by_offset(erased_node_parent, offset2, tree_root);
						}
						current_node = *tree_root;
						if (!current_node)
							return;
						resulting_parent = current_node->m_parent;
						break;
					}
					if (parent_subnode->right() && parent_subnode->right()->red())
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
					erased_node_parent = erased_node_parent->parent();
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
		rbnode_t** m_current_node_ptr;
		rbnode_t* m_current_parent;

		void insert(rbnode_t* new_node, rbnode_t** into_tree) {
			rbnode_t::link(new_node, m_current_parent, m_current_node_ptr);
			insert_node(new_node, into_tree);
		}
	};

	template<typename TContainer, uint32_t containing_record_offset, typename LambdaRightArgType, typename LambdaCompareType>
	CRSPTREE_FORCEINLINE
		static TContainer* find_entry_for_intrusive_tree(rbnode_t** root, LambdaRightArgType lambda_right_arg, LambdaCompareType&& compare_function, insertion_position_t* hint) {
		rbnode_t** iterator_position = root;

		rbnode_t* parent = nullptr;

		while (*iterator_position) {
			rbnode_t* current_node = *iterator_position;



			TContainer* container_of_node = reinterpret_cast<TContainer*>(
				reinterpret_cast<char*>(current_node) - containing_record_offset
				);

			ptrdiff_t sign_of_comparison = compare_function(container_of_node, lambda_right_arg);
			parent = *iterator_position;
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
}//namespace crsptree