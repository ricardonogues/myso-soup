#include "dtb.h"
#include "../common.h"
#include "../kernel.h"
#include <stdbool.h>

bool checkIsValidDTB(struct fdt_header *fdt) {
  uint32_t magic = bswap32(fdt->magic);

  if (magic != 0xd00dfeed) {
    return false;
  }

  return true;
}

char *searchDTB(struct fdt_header *fdt, char *prop) {
  if (!checkIsValidDTB(fdt)) {
    PANIC("NOT A VALID DTB");
  }

  uint32_t structsOffset = bswap32(fdt->off_dt_struct);
  uint32_t stringOffset = bswap32(fdt->off_dt_strings);
  uint32_t *token = (uint32_t *)(((char *)fdt) + structsOffset);

#ifdef DEBUG
  printf("STRUCTS ADDRESS: %x\n", (void *)(((char *)fdt) + structsOffset));
  printf("STRUCTS OFFSET: %d\n", structsOffset);
#endif

  uint32_t struct_end = structsOffset + bswap32(fdt->size_dt_struct);

  while ((char *)token < ((char *)fdt) + struct_end &&
         bswap32(*token) != FDT_END) {
#ifdef DEBUG
    printf("TOKEN: %x\n", bswap32(*token));
    printf("TOKEN: %x\n", token);
#endif
    switch (bswap32(*token)) {
    case FDT_BEGIN_NODE: {
      char *token_name = (char *)(token + 1);
      size_t name_length = strlen(token_name);
      if (name_length > 0) {
        printf("TOKEN NAME: %s\n", (void *)token_name);
      }
      token =
          (uint32_t *)(align_up((uint32_t)(token_name + name_length + 1), 4));

      break;
    }
    case FDT_END_NODE: {
      token += 1;
      break;
    }
    case FDT_PROP: {
      struct fdt_prop_info *info = (struct fdt_prop_info *)(token + 1);
      char *prop_name =
          ((char *)fdt) + stringOffset + bswap32(info->name_offset);

      printf("PROP NAME: %s\n", prop_name);

      if (!strcmp(prop_name, prop)) {
        return (char *)(info + 1);
      }
      token = (uint32_t *)(align_up(
          (uint32_t)(((char *)(info + 1)) + bswap32(info->length)), 4));
      break;
    }
    case FDT_NOP: {
      token += 1;
      break;
    }
    default:
      goto end;
    }
  }
end:
  return 0;
}
