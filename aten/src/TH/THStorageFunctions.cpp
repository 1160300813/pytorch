#include <climits>

#include "THStorageFunctions.hpp"

#include "generic/THStorage.cpp"
#include "THGenerateAllTypes.h"

#include "generic/THStorage.cpp"
#include "THGenerateHalfType.h"

#include "generic/THStorageCopy.cpp"
#include "THGenerateAllTypes.h"

#include "generic/THStorageCopy.cpp"
#include "THGenerateHalfType.h"

// Free a non-weak pointer to THStorage
void THStorage_free(THStorage* storage) {
  if (!storage) {
    return;
  }
  storage->release();
}

// Manually retains a weak reference
void THStorage_weakRetain(THStorage *weak_storage) {
  weak_storage->weak_retain();
}

// Releases a weak reference
void THStorage_weakFree(THStorage *weak_storage) {
  weak_storage->weak_release();
}

// Given a weak reference, returns a strong reference to a storage (which must
// be freed when done) or null if the storage is already dead.
THStorage* THStorage_weakLock(THStorage *weak_storage) {
  if (weak_storage->weak_lock())
    return weak_storage;
  return nullptr;
}

THDescBuff THLongStorage_sizeDesc(const THLongStorage *size) {
  return _THSizeDesc(THLongStorage_data(size), size->size());
}

THLongStorage *THLongStorage_newInferSize(THLongStorage *size, ptrdiff_t nElement)
{
  ptrdiff_t total_size = (size->size() > 0 ? 1 : 0);
  ptrdiff_t dim_infer = -1;
  ptrdiff_t i;
  for (i = 0; i < size->size(); i++) {
    if (THLongStorage_data(size)[i] == -1) {
      THArgCheck(dim_infer == -1, 1, "only one dimension can be inferred");
      dim_infer = i;
    } else {
      total_size *= THLongStorage_data(size)[i];
    }
  }
  if (dim_infer != -1) {
    THDescBuff buf = THLongStorage_sizeDesc(size);
    THArgCheck(total_size > 0 && nElement % total_size == 0, 2,
        "size '%s' is invalid for input with %td elements", buf.str, nElement);
  } else {
    THDescBuff buf = THLongStorage_sizeDesc(size);
    THArgCheck(nElement == total_size, 2,
        "size '%s' is invalid for input with %td elements", buf.str, nElement);
  }
  THLongStorage* copy = THLongStorage_newWithSize(size->size());
  THLongStorage_copy(copy, size);
  if (dim_infer != -1) {
    THLongStorage_data(copy)[dim_infer] = nElement / total_size;
  }
  return copy;
}

ptrdiff_t THStorage_size(const THStorage *self)
{
  return self->size();
}

void THStorage_retain(THStorage *storage)
{
  if (storage) {
    storage->retain();
  }
}

void THStorage_resize(THStorage* storage, ptrdiff_t size) {
  if (storage->resizable()) {
    /* case when the allocator does not have a realloc defined */
    at::DataPtr old_data;
    std::swap(old_data, storage->data_ptr());
    ptrdiff_t old_size = storage->size();
    if (size != 0) {
      storage->set_data_ptr(
          storage->allocator()->allocate(storage->elementSize() * size));
    }
    storage->set_size(size);
    if (old_data != nullptr) {
      ptrdiff_t copy_size = old_size;
      if (storage->size() < copy_size) {
        copy_size = storage->size();
      }
      if (copy_size > 0) {
        memcpy(
            storage->data(),
            old_data.get(),
            storage->elementSize() * copy_size);
      }
    }
  } else {
    THError("Trying to resize storage that is not resizable");
  }
}
