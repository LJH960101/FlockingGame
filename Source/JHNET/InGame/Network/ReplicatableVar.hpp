#pragma  once
#include "ReplicatableVar.h"

template<typename T>
bool CReplicatableVar<T>::operator!() const
{
	return !(this->_value);
}

template<typename T>
T& CReplicatableVar<T>::operator+=(const T& rhs)
{
	ChangeProcess(_value + rhs);
	return this->_value;
}

template<typename T>
T& CReplicatableVar<T>::operator-=(const T& rhs)
{
	ChangeProcess(_value - rhs);
	return this->_value;
}

template<typename T>
T& CReplicatableVar<T>::operator*=(const T& rhs)
{
	ChangeProcess(_value * rhs);
	return this->_value;
}

template<typename T>
T& CReplicatableVar<T>::operator/=(const T& rhs)
{
	ChangeProcess(_value / rhs);
	return this->_value;
}

template<typename T>
T& CReplicatableVar<T>::operator++()
{
	ChangeProcess(_value + 1);
	return _value;
}

template<typename T>
T CReplicatableVar<T>::operator++(int)
{
	T temp = _value;
	ChangeProcess(_value + 1);
	return temp;
}

template<typename T>
T& CReplicatableVar<T>::operator--()
{
	ChangeProcess(_value - 1);
	return _value;
}

template<typename T>
T CReplicatableVar<T>::operator--(int)
{
	T temp = _value;
	ChangeProcess(_value - 1);
	return temp;
}

// Replicatable T
template<typename T>
bool CReplicatableVar<T>::operator==(const CReplicatableVar<T>& rhs) const
{
	return this->_value == rhs._value;
}

template<typename T>
bool CReplicatableVar<T>::operator!=(const CReplicatableVar<T>& rhs) const
{
	return this->_value != rhs._value;
}

template<typename T>
bool CReplicatableVar<T>::operator<=(const CReplicatableVar<T>& rhs) const
{
	return this->_value <= rhs._value;
}

template<typename T>
bool CReplicatableVar<T>::operator>=(const CReplicatableVar<T>& rhs) const
{
	return this->_value >= rhs._value;
}

template<typename T>
bool CReplicatableVar<T>::operator<(const CReplicatableVar<T>& rhs) const
{
	return this->_value < rhs._value;
}

template<typename T>
bool CReplicatableVar<T>::operator>(const CReplicatableVar<T>& rhs) const
{
	return this->_value > rhs._value;
}

// T
template<typename T>
bool CReplicatableVar<T>::operator==(const T& rhs) const
{
	return this->_value == rhs;
}

template<typename T>
bool CReplicatableVar<T>::operator!=(const T& rhs) const
{
	return this->_value != rhs;
}

template<typename T>
bool CReplicatableVar<T>::operator<=(const T& rhs) const
{
	return this->_value <= rhs;
}

template<typename T>
bool CReplicatableVar<T>::operator>=(const T& rhs) const
{
	return this->_value >= rhs;
}

template<typename T>
bool CReplicatableVar<T>::operator<(const T& rhs) const
{
	return this->_value < rhs;
}

template<typename T>
bool CReplicatableVar<T>::operator>(const T& rhs) const
{
	return this->_value > rhs;
}

template<typename T>
CReplicatableVar<T>::CReplicatableVar() : _netBaseCP(nullptr)
{

}

template<typename T>
CReplicatableVar<T>::CReplicatableVar(const T& defaultValue, UNetworkBaseCP** netBaseCP) : _value(defaultValue), _netBaseCP(netBaseCP)
{
}

template<typename T>
CReplicatableVar<T>::~CReplicatableVar()
{
}

template<typename T>
CReplicatableVar<T>& CReplicatableVar<T>::operator=(const CReplicatableVar<T>& rhs)
{
	ChangeProcess(rhs._value);
	return *this;
}

template<typename T>
CReplicatableVar<T>& CReplicatableVar<T>::operator=(const T& rhs)
{
	ChangeProcess(rhs);
	return *this;
}

template<typename T>
T CReplicatableVar<T>::operator*()
{
	return _value;
}

template<typename T>
bool CReplicatableVar<T>::IsChanged(const T& newValue)
{
	if (newValue != _value) return true;
	else return false;
}

template<typename T>
inline void CReplicatableVar<T>::ChangeProcess(const T & newValue)
{
	if (_netBaseCP == nullptr) {
		if (_errorCounter >= MAX_ERROR_IGNORE) {
			LOG(Warning, "ReplicatableVar :: Dosen't have netbase. (_netBaseCP is null)");
		}
		else {
			++_errorCounter;
		}

		_value = newValue;
		return;
	}
	if (*_netBaseCP == nullptr) {
		if (_errorCounter >= MAX_ERROR_IGNORE) {
			LOG(Warning, "ReplicatableVar :: Dosen't have netbase's point. (*_netBaseCP is null)");
		}
		else {
			++_errorCounter;
		}

		_value = newValue;
		return;
	}
	if (IsChanged(newValue)) {
		// Something is change.
		if (_handle.IsEmpty() || !(*_netBaseCP)->CheckAlreadyUseSyncVar(_handle)) {
			if (_errorCounter >= MAX_ERROR_IGNORE) {
				LOG(Warning, "ReplicatableVar :: Not binding.");
			}
			else {
				++_errorCounter;
			}

			_value = newValue;
			return;
		}

		// SyncVar sync proc...
		if (!_handle.IsEmpty()) {
			(*_netBaseCP)->SetSyncVar(_handle, sizeof(newValue), (char*)&newValue, _bReliable);
		}
	}
}

template<typename T>
void CReplicatableVar<T>::ForceChange(const T& rhs)
{
	_value = rhs;
}

template<typename T>
bool CReplicatableVar<T>::_BindHandle(const FString& variableName)
{
	_handle = variableName;
	if (!(
		(*_netBaseCP)->BindSyncVar(_handle, sizeof(_value), (void*)&_value))
		) {
		_handle.Empty();
		LOG(Error, "ReplicatableVar :: Wrong handle... This change will not process.");
		return false;
	}
	return true;
}

template<typename T>
inline void CReplicatableVar<T>::Init(UNetworkBaseCP ** netBaseCP, const FString& variableName)
{
	_netBaseCP = netBaseCP;
	if(_netBaseCP!=nullptr) _BindHandle(variableName);
}