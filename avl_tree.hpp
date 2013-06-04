/******************************************************************************
 *                            Data Structure
 *                   Self-balancing binary tree data structure.
 *                   http://en.wikipedia.org/wiki/AVL_tree
 *****************************************************************************/

#ifndef _avl_TREE_HPP_
# define _avl_TREE_HPP_

# include <functional>

# undef DS

namespace DS {

  struct _avl_tree_node_base {
    _avl_tree_node_base* _parent;
    _avl_tree_node_base* _left;
    _avl_tree_node_base* _right;
    int                  _balance;
  };

  template <typename Data>
  struct _avl_tree_node : public _avl_tree_node_base {
    Data                      _value;
  };

  /**
   * Bidirectional iterator compatible with the STL
   */
  static
  _avl_tree_node_base*
  _avl_tree_decrement(_avl_tree_node_base* x) {
    if (x->_left != NULL) {
      _avl_tree_node_base* y = x->_left;
      while (y->_right != NULL)
        y = y->_right;
      x = y;
    } else {
      _avl_tree_node_base* y = x->_parent;
      while (x == y->_left) {
        x = y;
        y = y->_parent;
      }
      x = y;
    }
    return x;
  }

  static
  const _avl_tree_node_base*
  _avl_tree_decrement(const _avl_tree_node_base* x) {
    return _avl_tree_decrement(const_cast<_avl_tree_node_base*>(x));
  }

  static
  _avl_tree_node_base*
  _avl_tree_increment(_avl_tree_node_base* x) {
    if (x->_right != NULL) {
      x = x->_right;
      while (x->_left != NULL)
        x = x->_left;
    } else {
      _avl_tree_node_base* y = x->_parent;
      while (x == y->_right) {
        x = y;
        y = y->_parent;
      }
      if (x->_right != y)
        x = y;
    }
    return x;
  }

  static
  const _avl_tree_node_base*
  _avl_tree_increment(const _avl_tree_node_base* x) {
    return _avl_tree_increment(const_cast<_avl_tree_node_base*>(x));
  }

  template <typename Data>
  struct _avl_tree_iterator {
    typedef Data  value_type;
    typedef Data& reference;
    typedef Data* pointer;

    typedef std::bidirectional_iterator_tag iterator_category;
    typedef ptrdiff_t                       difference_type;

    typedef _avl_tree_iterator<Data> Self;
    typedef _avl_tree_node<Data>*    Link_type;

    _avl_tree_iterator()
    : _node() { }

    explicit _avl_tree_iterator(Link_type x)
    : _node(x) {}

    reference
    operator*() const
    { return static_cast<Link_type>(_node)->_value; }

    pointer
    operator->() const
    { return &static_cast<Link_type>(_node)->_value; }

    Self&
    operator++() {
      _node = _avl_tree_increment(_node);
      return *this;
    }

    Self
    operator++(int) {
      Self tmp = *this;
      _node = _avl_tree_increment(_node);
      return tmp;
    }

    Self&
    operator--() {
      _node = _avl_tree_decrement(_node);
      return *this;
    }

    Self
    operator--(int) {
      Self tmp = *this;
      _node = _avl_tree_decrement(_node);
      return tmp;
    }

    bool
    operator==(const Self& x) const
    { return _node == x._node; }

    bool
    operator!=(const Self& x) const
    { return _node != x._node; }

    _avl_tree_node_base* _node;
  };


  /**
   * Self-balancing binary search tree
   */
  namespace AVL {
    typedef _avl_tree_node_base* Node_ptr;

    struct rotation {
      static
      void 
      left(Node_ptr const x, 
           Node_ptr& root) {
        Node_ptr const y = x->_right;

        x->_right = y->_left;
        if (y->_left != NULL)
          y->_left->_parent = x;
        y->_parent = x->_parent;

        if (x == root)
          root = y;
        else if (x == x->_parent->_left)
          x->_parent->_left = y;
        else
          x->_parent->_right = y;
        y->_left = x;
        x->_parent = y;
      }

      static
      void 
      right(Node_ptr const x, 
            Node_ptr& root) {
        Node_ptr const y = x->_left;

        x->_left = y->_right;
        if (y->_right != NULL)
          y->_right->_parent = x;
        y->_parent = x->_parent;

        if (x == root)
          root = y;
        else if (x == x->_parent->_right)
          x->_parent->_right = y;
        else
          x->_parent->_left = y;
        y->_right = x;
        x->_parent = y;
      }
    };

    struct updater {
      static
      void
      update(Node_ptr leaf, Node_ptr& unbalanced, Node_ptr& root) {
        // Update balances bottom-up.
        for (;;) {
          Node_ptr p = leaf->_parent;
          if (p->_left == leaf)
            p->_balance--;
          else
            p->_balance++;
          if (p == unbalanced)
            break;
          leaf = p;
        }
      }
    };
  }

  template <typename Key,
            typename Data,
            typename Compare = std::less<Key>,
            typename Alloc = std::allocator<std::pair<const Key,Data> >,
            typename Rotation = AVL::rotation,
            typename Updater = AVL::updater>
  class avl_tree {
  public:
    typedef Key                                 key_type;
    typedef Data                                mapped_type;
    typedef std::pair<const Key,Data>           value_type;
    typedef value_type*                         pointer;
    typedef const value_type*                   const_pointer;
    typedef value_type&                         reference;
    typedef const value_type&                   const_reference;
    typedef size_t                              size_type;
    typedef Alloc                               allocator_type;

    typedef _avl_tree_iterator<value_type>      iterator;
    typedef std::reverse_iterator<iterator>     reverse_iterator;

  protected:
    typedef _avl_tree_node_base*              Base_ptr;
    typedef const _avl_tree_node_base*        Const_Base_ptr;
    typedef _avl_tree_node<value_type>*       Link_type;
    typedef const _avl_tree_node<value_type>* Const_Link_type;

  private:
    typedef typename Alloc::template rebind<_avl_tree_node<value_type> >::other
      _Node_allocator;
    _Node_allocator       _alloc;

  protected:
    Compare                    _compare;
    _avl_tree_node<value_type> _header; // Holds root, leftmost and rightmost nodes.
                                   // leftmost node of the tree, to enable constant time begin()
                                   // being parent of root enables representing end() iterator
                                   // having root for parent for finding root with a single indirection
    size_type                  _node_count; // Keeps track of size of tree.

  public:
    // allocation/deallocation
    avl_tree() {
      this->_header._left = &this->_header;
      this->_header._right = &this->_header;
      this->_header._parent = NULL;
      this->_header._balance = 0;
    }

    avl_tree(const avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>& o)
    : _alloc(o._alloc), _compare(o._compare), _header(o._header), _node_count(o._node_count) {
      if (o._header._parent != NULL) {
        _header._parent = _copy(o._begin(), _end());
        _header._parent->_parent = &_header;
        Base_ptr leftmost = _header._parent;
        while (leftmost->_left != NULL) leftmost = leftmost->_left;
        _header._left = leftmost;
        Base_ptr rightmost = _header._parent;
        while (rightmost->_right != NULL) rightmost = rightmost->_right;
        _header._right = rightmost;
      }
    }

    ~avl_tree() {
      _erase(_begin());
    }

    // Accessors.

    iterator begin() {
      return iterator(_left(&_header)); // points to leftmost (not to confuse with _begin)
    }
    iterator end() { return iterator(_end()); }

  protected:
    Link_type _begin() { return _parent(&_header); } // points to root
    Const_Link_type _begin() const { return _parent(&_header); }
    Link_type _end() { return static_cast<Link_type>(&_header); }
    Const_Link_type _end() const { return static_cast<Const_Link_type>(&_header); }

  public:
    // Set operations.
    iterator
    find(const key_type& k) {
      Link_type x = _begin(), y = _end();
      while (x != 0)
        if (!_compare(x->_value.first, k))
          y = x, x = _left(x);
        else
          x = _right(x);
      iterator j(y);
      return (j == end()
          || _compare(k,
            j->first)) ? end() : j;
    }

    std::pair<iterator,bool>
    insert(const value_type& v) {
      Link_type x = _begin();
      Link_type y = _end();
      Base_ptr unbalanced = y;
      bool comp = true;
      while (x != NULL) {
        if (x->_balance != 0)
          unbalanced = x;
        y = x;
        comp = _compare(v.first, x->_value.first);
        x = comp ? _left(x) : _right(x);
      }
      iterator j = iterator(y);
      if (comp) {
        if (j == begin())
          return std::make_pair(_insert(comp, y, v, unbalanced), true);
        else
          --j;
      }
      if (_compare(j->first, v.first))
        return std::make_pair(_insert(comp, y, v, unbalanced), true);
      return std::make_pair(j, false);
    }

  protected:
    static Link_type
    _left(Base_ptr x)
    { return static_cast<Link_type>(x->_left); }

    static Link_type
    _right(Base_ptr x)
    { return static_cast<Link_type>(x->_right); }

    static Link_type
    _parent(Base_ptr x)
    { return static_cast<Link_type>(x->_parent); }

    static Const_Link_type
    _left(Const_Base_ptr x)
    { return static_cast<Const_Link_type>(x->_left); }

    static Const_Link_type
    _right(Const_Base_ptr x)
    { return static_cast<Const_Link_type>(x->_right); }

    static Const_Link_type
    _parent(Const_Base_ptr x)
    { return static_cast<Const_Link_type>(x->_parent); }


  private:
    void
    _rebalance(Base_ptr&);

    iterator
    _insert(bool insert_left, Link_type&, const value_type&, Base_ptr&);

    void
    _erase(Link_type);

    Link_type
    _copy(Const_Link_type, Link_type);

    Link_type
    _clone_node(Const_Link_type x) {
      Link_type tmp = _alloc.allocate(1);
      Alloc(_alloc).construct(&tmp->_value, x->_value);
      tmp->_balance = x->_balance;
      tmp->_left = NULL;
      tmp->_right = NULL;
      return tmp;
    }
  };

  template <typename Key, typename Data, typename Compare, typename Alloc,
            typename Rotation, typename Updater>
  typename avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>::iterator
  avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>::_insert(bool insert_left,
      Link_type& p,
      const value_type& v,
      Base_ptr& unbalanced) {
    Link_type leaf = _alloc.allocate(1);
    Alloc(_alloc).construct(&leaf->_value, v);
    leaf->_parent = p;
    leaf->_left = NULL;
    leaf->_right = NULL;
    leaf->_balance = 0;

    // Insert.
    // Make new node child of parent and maintain root, leftmost and
    // rightmost nodes.
    // N.B. First node is always inserted left.
    if (p == _end() || insert_left) {
      p->_left = leaf;
      if (p == &_header) {
        _header._parent = leaf; // new root
        _header._right = leaf;
      } else if (p == _header._left)
        _header._left = leaf; // maintain leftmost pointing to min node
    } else {
      p->_right = leaf;

      if (p == _header._right)
        _header._right = leaf; // maintain rightmost pointing to max node
    }

    // Update balances bottom-up.
    Updater::update(leaf, unbalanced, _header._parent);

    // Rebalance the tree
    if (unbalanced != _end())
      _rebalance(unbalanced);

    _node_count++;
    return iterator(leaf);
  }

  template <typename Key, typename Data, typename Compare, typename Alloc,
            typename Rotation, typename Updater>
  void
  avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>::_rebalance(Base_ptr& unbalanced)
  {
    Base_ptr& root = _header._parent;

    // Rebalance.
    // After inserting a node, it is necessary to check each of the
    // node's ancestors for consistency with the rules of avl.
    switch (unbalanced->_balance) {
      case 1: case -1: case 0:
        break;
      case 2: {
        Base_ptr right = unbalanced->_right;

        if (right->_balance == 1) {
          unbalanced->_balance = 0;
          right->_balance = 0;
        } else {
          switch (right->_left->_balance) {
            case 1:
              unbalanced->_balance = -1;
              right->_balance = 0;
              break;
            case 0:
              unbalanced->_balance = 0;
              right->_balance = 0;
              break;
            case -1:
              unbalanced->_balance = 0;
              right->_balance = 1;
              break;
          }
          right->_left->_balance = 0;

          Rotation::right(right, root);
        }
        Rotation::left(unbalanced, root);
        break;
      }
      case -2: {
        Base_ptr left = unbalanced->_left;

        if (left->_balance == -1) {
          unbalanced->_balance = 0;
          left->_balance = 0;
        } else {
          switch (left->_right->_balance) {
            case 1:
              unbalanced->_balance = 0;
              left->_balance = -1;
              break;
            case 0:
              unbalanced->_balance = 0;
              left->_balance = 0;
              break;
            case -1:
              unbalanced->_balance = 1;
              left->_balance = 0;
              break;
          }
          left->_right->_balance = 0;

          Rotation::left(left, root);
        }
        Rotation::right(unbalanced, root);
        break;
      }
    }
  }

  template <typename Key, typename Data, typename Compare, typename Alloc,
            typename Rotation, typename Updater>
  void
  avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>::_erase(Link_type x) {
    // Erase without rebalancing.
    while (x != NULL) {
      _erase(_right(x));
      Link_type y = _left(x);
      Alloc(_alloc).destroy(&x->_value);
      _alloc.deallocate(x, 1);
      x = y;
    }
  }

  template <typename Key, typename Data, typename Compare, typename Alloc,
            typename Rotation, typename Updater>
  typename avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>::Link_type
  avl_tree<Key,Data,Compare,Alloc,Rotation,Updater>::_copy(Const_Link_type x,
                                                           Link_type p) {
    Link_type top = _clone_node(x);
    top->_parent = p;

    if (x->_right)
      top->_right = _copy(_right(x), top);
    p = top;
    x = _left(x);

    while (x != NULL) {
      Link_type y = _clone_node(x);
      p->_left = y;
      y->_parent = p;
      if (x->_right)
        y->_right = _copy(_right(x), y);
      p = y;
      x = _left(x);
    }
    return top;
  }

}

#endif
