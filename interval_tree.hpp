/******************************************************************************
 *                            Data Structure
 *                   Tree data structure for storing intervals.
 *                   http://en.wikipedia.org/wiki/Interval_tree
 *****************************************************************************/

#ifndef INTERVAL_TREE_HPP_
# define INTERVAL_TREE_HPP_

# include "avl_tree.hpp"

# undef DS

namespace DS {

  /**
   * Each node of the balanced binary search tree also contains
   * - the upper interval limit
   * - the maximal value among the high ends of the intervals
   * - the minimal value among the low ends of the intervals
   * rooted below the node.
   */
  template <typename Key, typename Data>
  struct interval_tree_value {
    Key max;
    Key min;
    Data data;
    operator Data() const { return data; }
  };

  /**
   * Order the intervals by increasing lower bounds.
   */
  template <typename Key, typename Compare>
  struct Interval_Compare {
    typedef std::pair<const Key,const Key> interval_type;

    bool operator()(const interval_type& x, const interval_type& y) const {
      return compare(x.first, y.first);
    }

  private:
    Compare compare;
  };

  /**
   * Overlapping predicate.
   */
  template <typename Key, typename Compare>
  struct Interval_overlap {
    typedef std::pair<const Key,const Key> interval_type;

    bool operator()(const interval_type& x, const interval_type& y) const {
      return compare(x.first, y.second) && compare(y.first, x.second);
    }

  private:
    Compare compare;
  };

  /**
   * Forward iterator compatible with the STL
   */
# define STACK_SIZE 64

  template <typename Tree>
  struct _interval_iterator {
    typedef typename Tree::Base_type::value_type Pointee;
    typedef Pointee  value_type;
    typedef Pointee& reference;
    typedef Pointee* pointer;

    typedef std::forward_iterator_tag       iterator_category;
    typedef ptrdiff_t                       difference_type;

    typedef _interval_iterator<Tree>          Self;
    typedef typename Pointee::first_type      interval_type;
    typedef _avl_tree_node<Pointee>*          Link_type;
    typedef typename Tree::Base_ptr           Base_ptr;

    _interval_iterator(Link_type root, const interval_type& i, Link_type e)
    : _interval(i), _end(e), _sp(0) {
      _stack[_sp++] = root;
      _forward();
    }

    reference
    operator*() const
    { return static_cast<Link_type>(_node)->_value; }

    pointer
    operator->() const
    { return &static_cast<Link_type>(_node)->_value; }

    Self&
    operator++() {
      _forward();
      return *this;
    }

    Self
    operator++(int) {
      Self tmp = *this;
      _forward();
      return tmp;
    }

    bool
    operator==(const Self& x) const
    { return _node == x._node; }

    bool
    operator!=(const Self& x) const
    { return _node != x._node; }

    void
    _forward() {
      Interval_overlap<typename Tree::key_type,
                       typename Tree::key_compare> _overlap;
      typename Tree::key_compare                   _compare;
      while (_sp > 0) {
        _node = _stack[--_sp];
        if (_node->_left != NULL
            && _compare(_interval.first, Tree::_left(_node)->_value.second.max))
          _stack[_sp++] = _node->_left;
        if (_node->_right != NULL
                 && _compare(Tree::_right(_node)->_value.second.min, _interval.second))
          _stack[_sp++] = _node->_right;
        if (_overlap(_interval, static_cast<Link_type>(_node)->_value.first))
          return;
      }
      _node = _end;
    }

    Base_ptr            _node;
    const interval_type _interval;
    Link_type           _end;
    Base_ptr            _stack[STACK_SIZE];
    int                 _sp;
  };

  template <typename Tree>
  struct _interval_const_iterator {
    typedef typename Tree::Base_type::value_type Pointee;
    typedef Pointee        value_type;
    typedef const Pointee& reference;
    typedef const Pointee* pointer;

    typedef std::forward_iterator_tag       iterator_category;
    typedef ptrdiff_t                       difference_type;

    typedef _interval_const_iterator<Tree>    Self;
    typedef typename Pointee::first_type      interval_type;
    typedef const _avl_tree_node<Pointee>*    Link_type;
    typedef typename Tree::Const_Base_ptr     Const_Base_ptr;

    _interval_const_iterator(Link_type root, const interval_type& i, Link_type e)
    : _interval(i), _end(e), _sp(0) {
      _stack[_sp++] = root;
      _forward();
    }

    _interval_const_iterator(const _interval_iterator<Tree>& it)
    : _node(it._node), _interval(it._interval), _end(it._end), _sp(it._sp) {
      for (int i = 0; i <_sp; ++i)
        _stack[i] = it._stack[i];
    }

    reference
    operator*() const
    { return static_cast<Link_type>(_node)->_value; }

    pointer
    operator->() const
    { return &static_cast<Link_type>(_node)->_value; }

    Self&
    operator++() {
      _forward();
      return *this;
    }

    Self
    operator++(int) {
      Self tmp = *this;
      _forward();
      return tmp;
    }

    bool
    operator==(const Self& x) const
    { return _node == x._node; }

    bool
    operator!=(const Self& x) const
    { return _node != x._node; }

    void
    _forward() {
      Interval_overlap<typename Tree::key_type,
                       typename Tree::key_compare> _overlap;
      typename Tree::key_compare                   _compare;
      while (_sp > 0) {
        _node = _stack[--_sp];
        if (_node->_left != NULL
            && _compare(_interval.first, Tree::_left(_node)->_value.second.max))
          _stack[_sp++] = _node->_left;
        if (_node->_right != NULL
                 && _compare(Tree::_right(_node)->_value.second.min, _interval.second))
          _stack[_sp++] = _node->_right;
        if (_overlap(_interval, static_cast<Link_type>(_node)->_value.first))
          return;
      }
      _node = _end;
    }

    Const_Base_ptr      _node;
    const interval_type _interval;
    Link_type           _end;
    Const_Base_ptr      _stack[STACK_SIZE];
    int                 _sp;
  };

  template <typename Tree>
  inline bool
  operator==(const _interval_iterator<Tree>& x,
             const _interval_const_iterator<Tree>& y)
  { return x._node == y._node; }

  template <typename Tree>
  inline bool
  operator!=(const _interval_iterator<Tree>& x,
             const _interval_const_iterator<Tree>& y)
  { return x._node != y._node; }


  /**
   * Augmented tree implementation.
   * Cormen et al. (2001, Section 14.3: Interval trees, pp. 311–317)
   *
   * Use a simple self-balancing binary search tree, where the tree is ordered
   * by the 'low' values of the intervals, and an extra annotation is added to
   * every node recording the maximum high value of both its subtrees.
   * It is simple to maintain this attribute in only O(h) steps during each
   * addition or removal of a node, where h is the height of the node added or
   * removed in the tree, by updating all ancestors of the node from the bottom
   * up.
   * Additionally, the tree rotations used during insertion and deletion
   * require updating the max value of the affected nodes.
   */
  template <typename Key,
            typename Data,
            typename Compare,
            typename Alloc>
  class interval_tree;

  namespace Interval {
    typedef _avl_tree_node_base*                  Node_ptr;

    template <typename Tree>
    struct rotation {
      static
      void
      left(Node_ptr x,
           Node_ptr& root) {
        AVL::rotation::left(x, root);
        _update_min_max(x);
      }

      static
      void
      right(Node_ptr x,
            Node_ptr& root) {
        AVL::rotation::right(x, root);
        _update_min_max(x);
      }

    private:
      static
      void
      _update_min_max(Node_ptr x) {
        typename Tree::Link_type node = static_cast<typename Tree::Link_type>(x);
        Tree::_parent(node)->_value.second.max = node->_value.second.max;
        Tree::_parent(node)->_value.second.min = node->_value.second.min;
        node->_value.second.max = node->_value.first.second;
        node->_value.second.min = node->_value.first.first;
        typename Tree::key_type& max_subtree = node->_value.second.max;
        typename Tree::key_type& min_subtree = node->_value.second.min;
        if (node->_left != NULL) {
          max_subtree = std::max(max_subtree, Tree::_left(node)->_value.second.max);
          min_subtree = std::min(min_subtree, Tree::_left(node)->_value.second.min);
        }
        if (node->_right != NULL) {
          max_subtree = std::max(max_subtree, Tree::_right(node)->_value.second.max);
          min_subtree = std::min(min_subtree, Tree::_right(node)->_value.second.min);
        }
      }
    };

    template <typename Tree>
    struct updater {
      static
      void
      update(Node_ptr x, Node_ptr& unbalanced, Node_ptr& root) {
        // Update balances bottom-up.
        bool unbalance_switch = false;
        typename Tree::Link_type leaf = static_cast<typename Tree::Link_type>(x);
        for (;;) {
          typename Tree::Link_type p = static_cast<typename Tree::Link_type>(leaf->_parent);
          if (!unbalance_switch) {
            if (p->_left == leaf)
              p->_balance--;
            else
              p->_balance++;
          }
          p->_value.second.max = std::max(leaf->_value.second.max, p->_value.second.max);
          p->_value.second.min = std::min(leaf->_value.second.min, p->_value.second.min);
          if (p == root) // up to the root
            break;
          if (p == unbalanced)
            unbalance_switch = true;
          leaf = p;
        }
      }
    };
  }

  template <typename Key,
            typename Data,
            typename Compare = std::less<Key>,
            typename Alloc = std::allocator<std::pair<
              const std::pair<const Key,const Key>,
              interval_tree_value<Key,Data> >
                > >
  class interval_tree : private avl_tree<std::pair<const Key,const Key>,
                                         interval_tree_value<Key,Data>,
                                         Interval_Compare<Key,Compare>,
                                         Alloc,
                                         Interval::rotation<interval_tree<Key,Data,Compare,Alloc> >,
                                         Interval::updater<interval_tree<Key,Data,Compare,Alloc> > >
  {
    template <typename Self> friend struct Interval::rotation;
    template <typename Self> friend struct Interval::updater;
    template <typename Self> friend struct _interval_iterator;
    template <typename Self> friend struct _interval_const_iterator;

    typedef avl_tree<std::pair<const Key,const Key>,
                     interval_tree_value<Key,Data>,
                     Interval_Compare<Key,Compare>,
                     Alloc,
                     Interval::rotation<interval_tree<Key,Data,Compare,Alloc> >,
                     Interval::updater<interval_tree<Key,Data,Compare,Alloc> > >
                                                    Base_type;
    typedef interval_tree<Key,Data,Compare,Alloc>   Self;
    typedef typename Base_type::Base_ptr            Base_ptr;
    typedef typename Base_type::Const_Base_ptr      Const_Base_ptr;
    typedef typename Base_type::Link_type           Link_type;
    typedef typename Base_type::Const_Link_type     Const_Link_type;

  public:
    typedef Key                                 key_type;
    typedef std::pair<const Key,const Key>      interval_type;
    typedef std::pair<const interval_type,Data> value_type;
    typedef Compare                             key_compare;
    typedef Interval_Compare<Key,Compare>       interval_compare;
    typedef _interval_iterator<Self>            iterator;
    typedef _interval_const_iterator<Self>      const_iterator;

  public:
    using Base_type::_header;

    interval_tree()
    : _end(&_header, _dummy_interval, &_header) {}

    interval_tree(const interval_tree<Key,Data,Compare,Alloc>& o)
    : Base_type(o), _end(&_header, _dummy_interval, &_header)
    {}

    /** 
     * Find all intervals containing the key k.
     */
    const_iterator
    equal_range(const key_type& k) const {
      interval_type i = std::make_pair(k, k);
      return equal_range(i);
    }

    /**
     * Find all intervals overlapping interval i.
     */
    const_iterator
    equal_range(const interval_type& i) const {
      Base_ptr root = this->_header._parent;
      return const_iterator(static_cast<Link_type>(root), i, &this->_header);
    }

    /**
     * Returns a pair, with its member pair::first set to an iterator pointing
     * to either the newly inserted interval or to the element that already had
     * its same interval in the map.
     * The pair::second element in the pair is set to true if a new element was
     * inserted or false if an element with the same key existed.
     */
    void
    insert(const value_type& x) {
      typename Base_type::mapped_type data;
      data.max = x.first.second;
      data.min = x.first.first;
      data.data = x.second;
      Base_type::insert(std::make_pair(x.first, data));
    }

    iterator
    end() {
      return _end;
    }

    const_iterator
    end() const {
      return _end;
    }
 

  private:
    interval_type                 _dummy_interval;
    iterator                      _end;
  };

}

#endif /* !INTERVAL_TREE_HXX_ */
