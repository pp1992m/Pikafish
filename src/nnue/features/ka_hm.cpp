/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2023 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Definition of input features HalfKAv2_hm of NNUE evaluation function

#include "ka_hm.h"

#include "../../position.h"

namespace Stockfish::Eval::NNUE::Features {

  // Index of a feature for a given king position and another piece on some square
  template<Color Perspective>
  inline IndexType KA_hm::make_index(Square s, Piece pc, int bucket) {
    return IndexType(IndexMap[(4 * bool(bucket >> 6)
                             + 2 * bool(Perspective == BLACK)
                             + bool(type_of(pc) == ADVISOR || type_of(pc) == BISHOP)) * SQUARE_NB
                             + s] + PieceSquareIndex[Perspective][pc] + PS_NB * (bucket & 63));
  }

  // Get the king bucket
  template<Color Perspective>
  uint8_t KA_hm::king_bucket(const Position& pos) {
    return KingBuckets[KingMaps[IndexMap[2 * bool(Perspective == BLACK) * SQUARE_NB + pos.square<KING>(WHITE)]]
                     + KingMaps[IndexMap[2 * bool(Perspective == BLACK) * SQUARE_NB + pos.square<KING>(BLACK)]]];
  }

  // Explicit template instantiations
  template uint8_t KA_hm::king_bucket<WHITE>(const Position& pos);
  template uint8_t KA_hm::king_bucket<BLACK>(const Position& pos);

  // Get a list of indices for active features
  template<Color Perspective>
  void KA_hm::append_active_indices(
    const Position& pos,
    IndexList& active
  ) {
    int bucket = king_bucket<Perspective>(pos);
    Bitboard bb = pos.pieces();
    while (bb)
    {
      Square s = pop_lsb(bb);
      active.push_back(make_index<Perspective>(s, pos.piece_on(s), bucket));
    }
  }

  // Explicit template instantiations
  template void KA_hm::append_active_indices<WHITE>(const Position& pos, IndexList& active);
  template void KA_hm::append_active_indices<BLACK>(const Position& pos, IndexList& active);

  // append_changed_indices() : get a list of indices for recently changed features
  template<Color Perspective>
  void KA_hm::append_changed_indices(
    uint8_t bucket,
    const DirtyPiece& dp,
    IndexList& removed,
    IndexList& added
  ) {
    for (int i = 0; i < dp.dirty_num; ++i) {
      if (dp.from[i] != SQ_NONE)
        removed.push_back(make_index<Perspective>(dp.from[i], dp.piece[i], bucket));
      if (dp.to[i] != SQ_NONE)
        added.push_back(make_index<Perspective>(dp.to[i], dp.piece[i], bucket));
    }
  }

  // Explicit template instantiations
  template void KA_hm::append_changed_indices<WHITE>(uint8_t bucket, const DirtyPiece& dp, IndexList& removed, IndexList& added);
  template void KA_hm::append_changed_indices<BLACK>(uint8_t bucket, const DirtyPiece& dp, IndexList& removed, IndexList& added);

  int KA_hm::update_cost(const StateInfo* st) {
    return st->dirtyPiece.dirty_num;
  }

  int KA_hm::refresh_cost(const Position& pos) {
    return pos.count<ALL_PIECES>();
  }

  bool KA_hm::requires_refresh(const StateInfo* st) {
    return type_of(st->dirtyPiece.piece[0]) == KING;
  }

}  // namespace Stockfish::Eval::NNUE::Features
