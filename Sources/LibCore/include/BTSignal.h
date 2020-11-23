/// https://github.com/Rominitch/MouCaLab
/// \author  Rominitch
/// \license No license
#pragma once

namespace Core
{
    template <typename... Args>
    class Signal
    {
        MOUCA_NOCOPY_NOMOVE(Signal);

        public:
            using IdConnexion = std::uintptr_t;

            Signal() = default;

            // connects a member function to this Signal
            template <typename ClassInstance>
            IdConnexion connectMember(ClassInstance* inst, void (ClassInstance::* func)(Args...))
            {
                return connect(reinterpret_cast<IdConnexion>(inst),
                               [=](Args... args)
                               {
                                   ( inst->*func )( args... );
                               });
            }

            // connects a const member function to this Signal
            template <typename ClassInstance>
            IdConnexion connectMember(ClassInstance* inst, void (ClassInstance::* func)(Args...) const)
            {
                return connect(reinterpret_cast<IdConnexion>(inst),
                               [=](Args... args)
                               {
                                   BT_ASSERT(inst != nullptr);
                                   (inst->*func)(args...);
                               });
            }

            // connects a member function to this Signal
            template <typename ClassInstance>
            IdConnexion connectWeakMember(std::weak_ptr<ClassInstance> inst, void ( ClassInstance::* func )( Args... ))
            {
                return connect(reinterpret_cast<IdConnexion>(inst.lock().get()),
                               [=](Args... args)
                               {
                                   BT_ASSERT(!inst.expired());
                                   ( inst.lock().get()->*func )( args... );
                               });
            }

            template <typename ClassInstance>
            IdConnexion connectWeakMember(std::weak_ptr<ClassInstance> inst, void ( ClassInstance::* func )( Args... ) const)
            {
                return connect(reinterpret_cast<IdConnexion>(inst.lock().get()),
                               [=](Args... args)
                               {
                                   BT_ASSERT(!inst.expired());
                                   ( inst.lock().get()->*func )( args... );
                               });
            }

            // disconnects a previously connected function
            template <typename ClassInstance>
            void disconnect(ClassInstance* pointer)
            {
                _slots.erase(reinterpret_cast<IdConnexion>(pointer));
            }

            // disconnects all previously connected functions
            void disconnectAll()
            {
                _slots.clear();
            }

            template <typename ClassInstance>
            bool isConnected(std::weak_ptr<ClassInstance> pointer) const
            {
                return isConnected(reinterpret_cast<IdConnexion>(pointer.lock().get()));
            }

            template <typename ClassInstance>
            bool isConnected(ClassInstance* pointer) const
            {
                return isConnected(reinterpret_cast<IdConnexion>(pointer));
            }

            // calls all connected functions
            void emit(Args... p) const
            {
                for(auto it : _slots)
                {
                    it.second(p...);
                }
            }

        private:
            // connects a std::function to the signal. The returned
            // value can be used to disconnect the function again
            //------------------------------------------------------------------------
            /// \brief  Generic connect system. WARNING: IdConnexion is NOT unique when pointer is delete often. It can provide issue if disconnect is not call after deletion.
            /// 
            /// \param[in] pointerID: current pointer.
            /// \param[in] slot: method to call.
            /// \returns Current pointer added.
            IdConnexion connect(IdConnexion pointerID, std::function<void(Args...)> const& slot)
            {
                BT_PRE_CONDITION(_slots.find(pointerID) == _slots.cend()); // DEV Issue: Must be unique ! This issue can happens if old pointer never call disconnect before this add (by multithreading ?).

                _slots.insert(std::make_pair(pointerID, slot));
                return pointerID;
            }

            bool isConnected(const IdConnexion pointerID) const
            {
                return _slots.find(pointerID) != _slots.cend();
            }

            std::map<IdConnexion, std::function<void(Args...)>> _slots; ///< List of all connected items.
    };
}