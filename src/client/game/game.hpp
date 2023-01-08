#pragma once

namespace game
{
  template <typename T>
  class symbol
  {
   public:
    symbol(const size_t mp) noexcept
        : mp_(reinterpret_cast<T*>(mp))
    {
    }

    T* get() const noexcept
    {
      return mp_;
    }

    void set(const size_t mp) noexcept
    {
      this->mp_ = reinterpret_cast<T*>(mp);
    }

    operator T*() const
    {
      return this->get();
    }

    T* operator->() const
    {
      return this->get();
    }

   private:
    T* mp_;
  };

  constexpr std::size_t MAX_CLIENTS = 18;
} // namespace game

#include "symbols.hpp"
