// Copyrightⓒ2019 by 블랙말랑카우. All Data cannot be copied without permission. 

#pragma once

#include "JHNET.h"
#include "Component/NetworkBaseCP.h"

#define MAX_ERROR_IGNORE 3

/**
 * ReplicatableVar
 * If you change values, then every client can get new value.
 */

template<typename T>
class JHNET_API CReplicatableVar
{
public:
	CReplicatableVar();
	CReplicatableVar(const T& defaultValue, UNetworkBaseCP** netBaseCP = nullptr);
	virtual ~CReplicatableVar();

	void Init(UNetworkBaseCP** netBaseCP, const FString& variableName);
	void SetReliable(const bool& bIsReliable) { _bReliable = bIsReliable; }

	// 네트워크 통신없이 강제로 바꾼다.
	void ForceChange(const T& rhs);

	// Operators
	CReplicatableVar<T>& operator=(const CReplicatableVar<T> &rhs);
	CReplicatableVar<T>& operator=(const T& rhs);
	bool operator!() const;
	T& operator++();
	T operator++(int);
	T& operator--();
	T operator--(int);
	T operator*();
	operator T() { return _value; }

	// Assignment Operators
	T& operator+=(const T& rhs);
	T& operator-=(const T& rhs);
	T& operator*=(const T& rhs);
	T& operator/=(const T& rhs);

	// Logical operators

	// Logical operator CReplicatableVar<T>
	bool operator==(const CReplicatableVar<T>& rhs) const;
	bool operator!=(const CReplicatableVar<T>& rhs) const;
	bool operator>=(const CReplicatableVar<T>& rhs) const;
	bool operator<=(const CReplicatableVar<T>& rhs) const;
	bool operator<(const CReplicatableVar<T>& rhs) const;
	bool operator>(const CReplicatableVar<T>& rhs) const;

	// Logical operator T
	bool operator==(const T& rhs) const;
	bool operator!=(const T& rhs) const;
	bool operator>=(const T& rhs) const;
	bool operator<=(const T& rhs) const;
	bool operator<(const T& rhs) const;
	bool operator>(const T& rhs) const;

	FString GetHandle() { return _handle; }

private:
	inline bool IsChanged(const T& newValue);
	void ChangeProcess(const T& newValue);
	bool _BindHandle(const FString& variableName);
	
	// MAX_ERROR_IGNORE번이상 값을 강제로 바꾸려고 하면 로그 출력을 위한 카운터
	uint8 _errorCounter = 0;

	FString _handle;
	UNetworkBaseCP** _netBaseCP =  nullptr;
	T _value;
	bool _bReliable = true;
};