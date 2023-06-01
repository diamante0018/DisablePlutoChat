#pragma once

namespace game
{
  [[nodiscard]] std::size_t get_base();
  [[nodiscard]] bool is_plutonium();

  template <typename T>
  class symbol
  {
   public:
    symbol(const std::size_t mp) noexcept
        : mp_(reinterpret_cast<T*>(mp))
    {
    }

    T* get() const noexcept
    {
      return mp_;
    }

    void set(const std::size_t mp) noexcept
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

#define _stdin_index 0
#define _printf(format, ...)                                 \
  {                                                          \
    auto* _stdin_file = &(game::__iob_func()[_stdin_index]); \
    game::_fprintf(_stdin_file, format, ##__VA_ARGS__);      \
  }

#include "symbols.hpp"
