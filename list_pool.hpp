#pragma once
#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

// Iterator Class
template <typename PT, typename N, typename T>
class _iterator {
	PT* pool;
	N current;

   public:
	using value_type = T;
	using reference = value_type&;
	using iterator_category = std::forward_iterator_tag;

	explicit _iterator(PT* p, N x) noexcept : pool{p}, current{x} {}

	reference operator*() const noexcept { return (*pool)[current - 1].value; }

	// pre-increment
	_iterator& operator++() {
		current = (*pool)[current - 1].next;
		return *this;
	}

	// post-increment
	_iterator operator++(int) {
		auto tmp = *this;
		++(*this);
		return tmp;
	}

	friend bool operator==(const _iterator& x, const _iterator& y) { return x.current = y.current; }

	friend bool operator!=(const _iterator& x, const _iterator& y) { return !(x == y); }
};

template <typename T, typename N = std::size_t>
class list_pool {
	struct node_t {
		T value;
		N next;

		// copy ctor
		node_t(const T& val, N nxt) noexcept : value{val}, next{nxt} {}

		// move ctor
		node_t(T&& val, N nxt) noexcept : value{std::move(val)}, next{nxt} {}
	};

	std::vector<node_t> pool;

	using list_type = N;
	using value_type = T;
	using size_type = typename std::vector<node_t>::size_type;

	list_type free_node_list = end();

	node_t& node(list_type x) noexcept { return pool[x - 1]; }

	const node_t& node(list_type x) const noexcept { return pool[x - 1]; }

   public:
	list_pool() noexcept = default;

	explicit list_pool(size_type n) { pool.reserve(n); }

	using iterator = _iterator<std::vector<node_t>*, list_type, value_type>;
	using const_iterator = _iterator<std::vector<node_t>*, list_type, const value_type>;

	// Iterators
	// add the space after clang-format in end()
	iterator begin(list_type x) { return iterator(&pool, x); }
	iterator end(list_type) { return iterator(&pool, end()); }
	const_iterator begin(list_type x) const { return const_iterator(&pool, x); }
	const_iterator end(list_type) const { return const_iterator(&pool, end()); }
	const_iterator cbegin(list_type x) const { return const_iterator(&pool, x); }
	const_iterator cend(list_type) const { return const_iterator(&pool, end()); }

	list_type new_list() noexcept { return end(); }

	void reserve(size_type n) { pool.reserve(n); }

	size_type capacity() const noexcept { return pool.capacity(); }

	bool is_empty(list_type x) const noexcept { return x == end(); }

	list_type end() const noexcept { return list_type(0); }

	T& value(list_type x) noexcept { return node(x).value; }
	const T& value(list_type x) const noexcept { return node(x).value; }

	list_type& next(list_type x) { return node(x).next; }
	const list_type& next(list_type x) const { return node(x).next; }

	list_type _push_front(value_type&& val, list_type head) {
		if (free_node_list) {
			auto head_new = free_node_list;
			free_node_list = node(head_new).next;
			node(head_new).value = std::forward<value_type>(val);
			if (is_empty(head)) {
				next(head_new) = end();
			} else {
				next(head_new) = head;
			}
			return head_new;
		} else {
			if (is_empty(head)) {
				pool.emplace_back(std::forward<value_type>(val), end());
			} else {
				pool.emplace_back(std::forward<value_type>(val), head);
				return pool.size();
			}
			return head;
		}
	}

	list_type push_front(const T& val, list_type head) { return _push_front(val, head); }

	list_type push_front(T&& val, list_type head) { return _push_front(std::move(val), head); }

	list_type get_last_node(list_type head) {
		list_type tmp = head;
		while (next(tmp) != end()) {
			tmp = next(tmp);
		}
		return tmp;
	}

	list_type _push_back(value_type&& val, list_type head) {
		if (is_empty(head)) {
			return push_front(std::forward<value_type>(val), head);
		}
		auto last = get_last_node(head);
		if (free_node_list) {
			auto tail_new = free_node_list;
			free_node_list = node(tail_new).next;
			node(tail_new).value = std::forward<value_type>(val);
			next(tail_new) = end();
			next(last) = tail_new;
		} else {
			pool.emplace_back(std::forward<value_type>(val), end());
			next(last) = pool.size();
		}
		return head;
	}

	list_type push_back(T&& val, list_type head) { return _push_back(std::move(val), head); }

	list_type push_back(const T& val, list_type head) { return _push_back(val, head); }

	list_type free(list_type x) {
		if (x == end())
			return x;
		auto tmp = node(x).next;
		node(x).next = free_node_list;
		free_node_list = x;
		return tmp;
	}

	list_type free_list(list_type x) {
		while (x != end()) {
			x = free(x);
		}
		return x;
	}
};
